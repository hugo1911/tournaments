#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <crow.h>
#include <nlohmann/json.hpp>
#include "domain/Match.hpp"
#include "controller/MatchController.hpp"
#include "delegate/IMatchDelegate.hpp"

class MatchDelegateMock : public IMatchDelegate {
public:
  MOCK_METHOD((std::expected<std::shared_ptr<domain::Match>, Error>), GetMatch,
              (std::string_view tournamentId, std::string_view matchId), (override));
  MOCK_METHOD((std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>), GetMatches,
              (std::string_view tournamentId), (override));
  MOCK_METHOD((std::expected<std::string, Error>), UpdateMatchScore, (const domain::Match&), (override));
};

class MatchControllerTest : public ::testing::Test {
protected:
  std::shared_ptr<MatchDelegateMock> matchDelegateMock;
  std::shared_ptr<MatchController> matchController;

  void SetUp() override {
    matchDelegateMock = std::make_shared<MatchDelegateMock>();
    matchController = std::make_shared<MatchController>(matchDelegateMock);
  }
};

// ============================================================================
// Tests de GetMatches - GET /tournaments/<TOURNAMENT_ID>/matches
// ============================================================================

// Validar respuesta exitosa con lista de matches. Response 200
TEST_F(MatchControllerTest, GetMatches_Ok) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  auto match1 = std::make_shared<domain::Match>();
  match1->Id() = "match-1";
  match1->TournamentId() = tournamentId;
  match1->HomeTeamId() = "team-1";
  match1->VisitorTeamId() = "team-2";
  match1->Name() = "W0";

  auto match2 = std::make_shared<domain::Match>();
  match2->Id() = "match-2";
  match2->TournamentId() = tournamentId;
  match2->HomeTeamId() = "team-3";
  match2->VisitorTeamId() = "team-4";
  match2->Name() = "W1";
  match2->MatchScore().homeTeamScore = 1;
  match2->MatchScore().visitorTeamScore = 2;

  std::vector<std::shared_ptr<domain::Match>> matches = {match1, match2};

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(matches)));

  auto response = matchController->getMatches(crow::request(), tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  EXPECT_EQ(jsonResponse.size(), 2);
}

// Validar respuesta exitosa con lista vacia. Response 200
TEST_F(MatchControllerTest, GetMatches_Empty) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::vector<std::shared_ptr<domain::Match>> emptyMatches;

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(emptyMatches)));

  auto response = matchController->getMatches(crow::request(), tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  EXPECT_EQ(jsonResponse.size(), 0);
}

// Validar respuesta NOT_FOUND cuando el torneo no existe. Response 404
TEST_F(MatchControllerTest, GetMatches_TournamentNotFound) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440001";

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(std::unexpected(Error::NOT_FOUND))));

  auto response = matchController->getMatches(crow::request(), tournamentId);

  EXPECT_EQ(response.code, crow::NOT_FOUND);
}

// Validar respuesta INTERNAL_SERVER_ERROR en caso de error de sistema. Response 500
TEST_F(MatchControllerTest, GetMatches_InternalServerError) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(std::unexpected(Error::UNKNOWN_ERROR))));

  auto response = matchController->getMatches(crow::request(), tournamentId);

  EXPECT_EQ(response.code, crow::INTERNAL_SERVER_ERROR);
}

// ============================================================================
// Tests de GetMatch - GET /tournaments/<TOURNAMENT_ID>/matches/<MATCH_ID>
// ============================================================================

// Validar respuesta exitosa y contenido completo. Response 200
TEST_F(MatchControllerTest, GetMatch_Ok) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  auto match = std::make_shared<domain::Match>();
  match->Id() = matchId;
  match->TournamentId() = tournamentId;
  match->HomeTeamId() = "team-1";
  match->VisitorTeamId() = "team-2";
  match->Name() = "W0";
  match->MatchScore().homeTeamScore = 3;
  match->MatchScore().visitorTeamScore = 2;

  EXPECT_CALL(*matchDelegateMock, GetMatch(tournamentId, matchId))
      .WillOnce(testing::Return(std::expected<std::shared_ptr<domain::Match>, Error>(match)));

  auto response = matchController->getMatch(tournamentId, matchId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  EXPECT_EQ(jsonResponse["id"], matchId);
}

// Validar respuesta NOT_FOUND cuando el partido no existe. Response 404
TEST_F(MatchControllerTest, GetMatch_NotFound) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "non-existent-match";

  EXPECT_CALL(*matchDelegateMock, GetMatch(tournamentId, matchId))
      .WillOnce(testing::Return(std::expected<std::shared_ptr<domain::Match>, Error>(std::unexpected(Error::NOT_FOUND))));

  auto response = matchController->getMatch(tournamentId, matchId);

  EXPECT_EQ(response.code, crow::NOT_FOUND);
}

// Validar respuesta INTERNAL_SERVER_ERROR en caso de error de sistema. Response 500
TEST_F(MatchControllerTest, GetMatch_InternalServerError) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  EXPECT_CALL(*matchDelegateMock, GetMatch(tournamentId, matchId))
      .WillOnce(testing::Return(std::expected<std::shared_ptr<domain::Match>, Error>(std::unexpected(Error::UNKNOWN_ERROR))));

  auto response = matchController->getMatch(tournamentId, matchId);

  EXPECT_EQ(response.code, crow::INTERNAL_SERVER_ERROR);
}

// ============================================================================
// Tests de UpdateMatchScore - PATCH /tournaments/<TOURNAMENT_ID>/matches/<MATCH_ID>
// ============================================================================

// Validar actualizacion exitosa del marcador. Response 204 (NO_CONTENT)
TEST_F(MatchControllerTest, UpdateMatchScore_Ok) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["score"]["home"] = 3;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  EXPECT_CALL(*matchDelegateMock, UpdateMatchScore(testing::_))
      .WillOnce(testing::Return(std::expected<std::string, Error>(matchId)));

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::NO_CONTENT);
}

// Validar respuesta NOT_FOUND cuando el partido no existe. Response 404
TEST_F(MatchControllerTest, UpdateMatchScore_MatchNotFound) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "non-existent-match";

  nlohmann::json jsonBody;
  jsonBody["score"]["home"] = 3;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  EXPECT_CALL(*matchDelegateMock, UpdateMatchScore(testing::_))
      .WillOnce(testing::Return(std::expected<std::string, Error>(std::unexpected(Error::NOT_FOUND))));

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::NOT_FOUND);
}

// Validar respuesta INTERNAL_SERVER_ERROR en caso de error de sistema. Response 500
TEST_F(MatchControllerTest, UpdateMatchScore_InternalServerError) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["score"]["home"] = 3;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  EXPECT_CALL(*matchDelegateMock, UpdateMatchScore(testing::_))
      .WillOnce(testing::Return(std::expected<std::string, Error>(std::unexpected(Error::UNKNOWN_ERROR))));

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::INTERNAL_SERVER_ERROR);
}

// Validar formato JSON invalido. Response 400 (BAD_REQUEST)
TEST_F(MatchControllerTest, UpdateMatchScore_InvalidJSON) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  crow::request request;
  request.body = "{invalid json}";

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "Invalid JSON format");
}

// Validar que falte el objeto score. Response 400
TEST_F(MatchControllerTest, UpdateMatchScore_MissingScore) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["someOtherField"] = "value";

  crow::request request;
  request.body = jsonBody.dump();

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "Missing or invalid score object");
}

// Validar que los scores sean enteros. Response 400
TEST_F(MatchControllerTest, UpdateMatchScore_InvalidScoreType) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["score"]["home"] = "three"; // string instead of int
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "score must contain integer home and visitor");
}

// Validar que los scores sean no negativos. Response 400
TEST_F(MatchControllerTest, UpdateMatchScore_NegativeScore) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["score"]["home"] = -1;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "Scores must be non-negative");
}

// Validar que el tournamentId en el body coincida con el path. Response 400
TEST_F(MatchControllerTest, UpdateMatchScore_TournamentIdMismatch) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["tournamentId"] = "different-tournament-id";
  jsonBody["score"]["home"] = 3;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "Tournament ID in body does not match path");
}

// Validar que el matchId en el body coincida con el path. Response 400
TEST_F(MatchControllerTest, UpdateMatchScore_MatchIdMismatch) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
  std::string matchId = "match-123";

  nlohmann::json jsonBody;
  jsonBody["id"] = "different-match-id";
  jsonBody["score"]["home"] = 3;
  jsonBody["score"]["visitor"] = 2;

  crow::request request;
  request.body = jsonBody.dump();

  auto response = matchController->updateMatchScore(request, tournamentId, matchId);

  EXPECT_EQ(response.code, crow::BAD_REQUEST);
  EXPECT_EQ(response.body, "Match ID in body does not match path");
}
// ============================================================================
// Tests de Query Parameters - Filtros showMatches
// ============================================================================

// Validar filtro showMatches=played - solo devuelve matches jugados
TEST_F(MatchControllerTest, GetMatches_FilterPlayed) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  // Match 1: jugado (con score)
  auto match1 = std::make_shared<domain::Match>();
  match1->Id() = "660e8400-e29b-41d4-a716-446655440001";
  match1->TournamentId() = tournamentId;
  match1->MatchScore().homeTeamScore = 3;
  match1->MatchScore().visitorTeamScore = 2;

  // Match 2: pendiente (sin score, 0-0)
  auto match2 = std::make_shared<domain::Match>();
  match2->Id() = "770e8400-e29b-41d4-a716-446655440002";
  match2->TournamentId() = tournamentId;
  match2->MatchScore().homeTeamScore = 0;
  match2->MatchScore().visitorTeamScore = 0;

  // Match 3: jugado (con score)
  auto match3 = std::make_shared<domain::Match>();
  match3->Id() = "880e8400-e29b-41d4-a716-446655440003";
  match3->TournamentId() = tournamentId;
  match3->MatchScore().homeTeamScore = 1;
  match3->MatchScore().visitorTeamScore = 0;

  std::vector<std::shared_ptr<domain::Match>> matches = {match1, match2, match3};

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(matches)));

  // Create request with query parameter
  crow::request request;
  request.url = "/tournaments/" + tournamentId + "/matches?showMatches=played";
  request.url_params = crow::query_string("?showMatches=played");

  auto response = matchController->getMatches(request, tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  // Should only return 2 matches (match1 and match3, not match2 which is 0-0)
  EXPECT_EQ(jsonResponse.size(), 2);
}

// Validar filtro showMatches=pending - solo devuelve matches pendientes
TEST_F(MatchControllerTest, GetMatches_FilterPending) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  // Match 1: jugado (con score)
  auto match1 = std::make_shared<domain::Match>();
  match1->Id() = "660e8400-e29b-41d4-a716-446655440001";
  match1->TournamentId() = tournamentId;
  match1->MatchScore().homeTeamScore = 3;
  match1->MatchScore().visitorTeamScore = 2;

  // Match 2: pendiente (sin score, 0-0)
  auto match2 = std::make_shared<domain::Match>();
  match2->Id() = "770e8400-e29b-41d4-a716-446655440002";
  match2->TournamentId() = tournamentId;
  match2->MatchScore().homeTeamScore = 0;
  match2->MatchScore().visitorTeamScore = 0;

  // Match 3: pendiente (sin score, 0-0)
  auto match3 = std::make_shared<domain::Match>();
  match3->Id() = "880e8400-e29b-41d4-a716-446655440003";
  match3->TournamentId() = tournamentId;
  match3->MatchScore().homeTeamScore = 0;
  match3->MatchScore().visitorTeamScore = 0;

  std::vector<std::shared_ptr<domain::Match>> matches = {match1, match2, match3};

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(matches)));

  // Create request with query parameter
  crow::request request;
  request.url = "/tournaments/" + tournamentId + "/matches?showMatches=pending";
  request.url_params = crow::query_string("?showMatches=pending");

  auto response = matchController->getMatches(request, tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  // Should only return 2 matches (match2 and match3, not match1 which has score)
  EXPECT_EQ(jsonResponse.size(), 2);
}

// Validar sin filtro - devuelve todos los matches
TEST_F(MatchControllerTest, GetMatches_NoFilter) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  auto match1 = std::make_shared<domain::Match>();
  match1->Id() = "660e8400-e29b-41d4-a716-446655440001";
  match1->TournamentId() = tournamentId;
  match1->MatchScore().homeTeamScore = 3;
  match1->MatchScore().visitorTeamScore = 2;

  auto match2 = std::make_shared<domain::Match>();
  match2->Id() = "770e8400-e29b-41d4-a716-446655440002";
  match2->TournamentId() = tournamentId;
  match2->MatchScore().homeTeamScore = 0;
  match2->MatchScore().visitorTeamScore = 0;

  std::vector<std::shared_ptr<domain::Match>> matches = {match1, match2};

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(matches)));

  // Create request without query parameter
  crow::request request;
  request.url = "/tournaments/" + tournamentId + "/matches";

  auto response = matchController->getMatches(request, tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  // Should return all matches
  EXPECT_EQ(jsonResponse.size(), 2);
}

// Validar filtro invalido - devuelve todos los matches
TEST_F(MatchControllerTest, GetMatches_InvalidFilter) {
  std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";

  auto match1 = std::make_shared<domain::Match>();
  match1->Id() = "660e8400-e29b-41d4-a716-446655440001";
  match1->TournamentId() = tournamentId;

  std::vector<std::shared_ptr<domain::Match>> matches = {match1};

  EXPECT_CALL(*matchDelegateMock, GetMatches(tournamentId))
      .WillOnce(testing::Return(std::expected<std::vector<std::shared_ptr<domain::Match>>, Error>(matches)));

  // Create request with invalid filter value
  crow::request request;
  request.url = "/tournaments/" + tournamentId + "/matches?showMatches=invalid";
  request.url_params = crow::query_string("?showMatches=invalid");

  auto response = matchController->getMatches(request, tournamentId);

  EXPECT_EQ(response.code, crow::OK);
  auto jsonResponse = nlohmann::json::parse(response.body);
  // Should return all matches (invalid filter is ignored)
  EXPECT_EQ(jsonResponse.size(), 1);
}
