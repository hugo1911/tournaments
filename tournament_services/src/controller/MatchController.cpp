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

  // Validate tournamentId if present in body
  if (requestBody.contains("tournamentId")) {
    std::string bodyTournamentId = requestBody["tournamentId"].get<std::string>();
    if (bodyTournamentId != tournamentId) {
      response.code = crow::BAD_REQUEST;
      response.body = "Tournament ID in body does not match path";
      return response;
    }
  }

  // Validate matchId if present in body
  if (requestBody.contains("id")) {
    std::string bodyMatchId = requestBody["id"].get<std::string>();
    if (bodyMatchId != matchId) {
      response.code = crow::BAD_REQUEST;
      response.body = "Match ID in body does not match path";
      return response;
    }
  }

  // Basic JSON shape validation for score
  if (!requestBody.contains("score") || !requestBody["score"].is_object()) {
    response.code = crow::BAD_REQUEST;
    response.body = "Missing or invalid score object";
    return response;
  }

  auto& scoreJson = requestBody["score"];
  if (!scoreJson.contains("home") || !scoreJson.contains("visitor") ||
      !scoreJson["home"].is_number_integer() || !scoreJson["visitor"].is_number_integer()) {
    response.code = crow::BAD_REQUEST;
    response.body = "score must contain integer home and visitor";
    return response;
  }

  int home = scoreJson["home"].get<int>();
  int visitor = scoreJson["visitor"].get<int>();
  if (home < 0 || visitor < 0) {
    response.code = crow::BAD_REQUEST;
    response.body = "Scores must be non-negative";
    return response;
  }

  // Create Match object with score
  domain::Match matchObj;
  matchObj.TournamentId() = tournamentId;
  matchObj.Id() = matchId;
  matchObj.MatchScore().homeTeamScore = home;
  matchObj.MatchScore().visitorTeamScore = visitor;

  auto res = matchDelegate->UpdateMatchScore(matchObj);
  if (res) {
    response.code = crow::NO_CONTENT; // HTTP 204
  } else {
    response.code = mapErrorToStatus(res.error());
    response.body = "Error";
  }

  return response;
}

REGISTER_ROUTE(MatchController, getMatches, "/tournaments/<string>/matches", "GET"_method)
REGISTER_ROUTE(MatchController, getMatch, "/tournaments/<string>/matches/<string>", "GET"_method)
REGISTER_ROUTE(MatchController, updateMatchScore, "/tournaments/<string>/matches/<string>", "PATCH"_method)
