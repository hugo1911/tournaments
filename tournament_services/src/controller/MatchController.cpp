#define JSON_CONTENT_TYPE "application/json"
#define CONTENT_TYPE_HEADER "content-type"

#include "configuration/RouteDefinition.hpp"
#include "controller/MatchController.hpp"

#include "configuration/RouteDefinition.hpp"
#include "domain/Utilities.hpp"
#include "exception/Error.hpp"
#include <iostream>

MatchController::MatchController(const std::shared_ptr<IMatchDelegate>& matchDelegate) : matchDelegate(matchDelegate) {}

static int mapErrorToStatus(const Error err) {
  switch (err) {
    case Error::NOT_FOUND: return crow::NOT_FOUND;
    case Error::INVALID_FORMAT: return crow::BAD_REQUEST;
    case Error::DUPLICATE: return crow::CONFLICT;
    default: return crow::INTERNAL_SERVER_ERROR;
  }
}


crow::response MatchController::getMatches(const crow::request& request, const std::string& tournamentId) {
  auto res = matchDelegate->GetMatches(tournamentId);
  if (res) {
    // Get query parameter for filtering
    std::string showMatches = request.url_params.get("showMatches") ? request.url_params.get("showMatches") : "";

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& matchptr : *res) {
      if (matchptr) {
        // Apply filter based on query parameter
        bool includeMatch = true;

        if (showMatches == "played") {
          // Only include matches that have been played (have a score)
          // A match is "played" if at least one team has a non-zero score
          includeMatch = (matchptr->MatchScore().homeTeamScore != 0 || matchptr->MatchScore().visitorTeamScore != 0);
        } else if (showMatches == "pending") {
          // Only include matches that haven't been played yet (no score)
          // A match is pending if both scores are 0 (default/unplayed)
          includeMatch = (matchptr->MatchScore().homeTeamScore == 0 && matchptr->MatchScore().visitorTeamScore == 0);
        }
        // If no filter or unknown filter, include all matches

        if (includeMatch) {
          arr.push_back(*matchptr);
        }
      } else {
        if (showMatches.empty()) {
          arr.push_back(nullptr);
        }
      }
    }
    auto response = crow::response{crow::OK, arr.dump()};
    response.add_header(CONTENT_TYPE_HEADER, JSON_CONTENT_TYPE);
    return response;
  } else {
    return crow::response{ mapErrorToStatus(res.error())};
  }
}

crow::response MatchController::getMatch(const std::string& tournamentId, const std::string& matchId) {
  auto res = matchDelegate->GetMatch(tournamentId, matchId);
  if (res) {
    nlohmann::json json = *(*res);
    auto response = crow::response{crow::OK, json.dump()};
    response.add_header(CONTENT_TYPE_HEADER, JSON_CONTENT_TYPE);
    return response;
  } else {
    return crow::response{ mapErrorToStatus(res.error())};
  }
}

crow::response MatchController::updateMatchScore(const crow::request& request, const std::string& tournamentId, const std::string& matchId) {
  crow::response response;
  if (!nlohmann::json::accept(request.body)) {
    response.code = crow::BAD_REQUEST;
    response.body = "Invalid JSON format";
    return response;
  }

  auto requestBody = nlohmann::json::parse(request.body);

  // Basic JSON shape validation for matchScore
  if (!requestBody.contains("matchScore") || !requestBody["matchScore"].is_object()) {
    response.code = crow::BAD_REQUEST;
    response.body = "Missing or invalid matchScore object";
    return response;
  }

  auto& scoreJson = requestBody["matchScore"];
  if (!scoreJson.contains("homeTeamScore") || !scoreJson.contains("visitorTeamScore") ||
      !scoreJson["homeTeamScore"].is_number_integer() || !scoreJson["visitorTeamScore"].is_number_integer()) {
    response.code = crow::BAD_REQUEST;
    response.body = "matchScore must contain integer homeTeamScore and visitorTeamScore";
    return response;
  }

  int home = scoreJson["homeTeamScore"].get<int>();
  int visitor = scoreJson["visitorTeamScore"].get<int>();
  if (home < 0 || visitor < 0) {
    response.code = crow::BAD_REQUEST;
    response.body = "Scores must be non-negative";
    return response;
  }

  // Deserialize into domain::Match (assumes nlohmann conversion exists)
  domain::Match matchObj = requestBody;

  // Ensure tournamentId matches path or is filled
  if (!matchObj.TournamentId().empty() && matchObj.TournamentId() != tournamentId) {
    response.code = crow::BAD_REQUEST;
    response.body = "Tournament ID in body does not match path";
    return response;
  }
  matchObj.TournamentId() = tournamentId;

  // Allow empty ID (client didn't set it) or ID equal to path; reject otherwise
  if (!matchObj.Id().empty() && matchObj.Id() != matchId) {
    response.code = crow::BAD_REQUEST;
    response.body = "Match ID in body does not match path";
    return response;
  }
  matchObj.Id() = matchId;

  auto res = matchDelegate->UpdateMatchScore(matchObj);
  if (res) {
    response.code = crow::OK;
    response.body = *res;
    response.add_header(CONTENT_TYPE_HEADER, JSON_CONTENT_TYPE);
  } else {
    response.code = mapErrorToStatus(res.error());
    response.body = "Error";
  }

  return response;
}

REGISTER_ROUTE(MatchController, getMatches, "/tournaments/<string>/matches", "GET"_method)
REGISTER_ROUTE(MatchController, getMatch, "/tournaments/<string>/matches/<string>", "GET"_method)
REGISTER_ROUTE(MatchController, updateMatchScore, "/tournaments/<string>/matches/<string>", "PATCH"_method)
