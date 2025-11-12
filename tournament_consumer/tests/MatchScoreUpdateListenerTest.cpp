#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "event/ScoreUpdateEvent.hpp"

// Tests para el MatchScoreUpdateListener
// Estos tests verifican la serialización y deserialización de mensajes ScoreUpdateEvent
class MatchScoreUpdateListenerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup básico para tests
    }
};

// ============================================================================
// Tests de procesamiento de mensajes validos
// ============================================================================

// Test: Validar que un mensaje JSON valido se procese correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_ValidJSON) {
    // Crear un mensaje JSON valido
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-123";
    messageJson["homeTeamScore"] = 3;
    messageJson["visitorTeamScore"] = 2;

    std::string message = messageJson.dump();

    // Este test verifica que el mensaje se parsea sin lanzar excepciones
    // En un escenario real, mockearíamos el MatchDelegate y verificaríamos
    // que ProcessScoreUpdate se llama con los datos correctos

    // Por ahora, simplemente verificamos que el JSON es válido
    EXPECT_NO_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

        EXPECT_EQ(event.tournamentId, "550e8400-e29b-41d4-a716-446655440000");
        EXPECT_EQ(event.matchId, "match-123");
        EXPECT_EQ(event.homeTeamScore, 3);
        EXPECT_EQ(event.visitorTeamScore, 2);
    });
}

// Test: Validar que un mensaje con scores de 0-0 se procese correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_ZeroScores) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-456";
    messageJson["homeTeamScore"] = 0;
    messageJson["visitorTeamScore"] = 0;

    std::string message = messageJson.dump();

    EXPECT_NO_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

        EXPECT_EQ(event.homeTeamScore, 0);
        EXPECT_EQ(event.visitorTeamScore, 0);
    });
}

// Test: Validar que un mensaje con scores altos se procese correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_HighScores) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-789";
    messageJson["homeTeamScore"] = 100;
    messageJson["visitorTeamScore"] = 99;

    std::string message = messageJson.dump();

    EXPECT_NO_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

        EXPECT_EQ(event.homeTeamScore, 100);
        EXPECT_EQ(event.visitorTeamScore, 99);
    });
}

// ============================================================================
// Tests de manejo de errores
// ============================================================================

// Test: Validar que un JSON invalido se maneje correctamente (no debe crashear)
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_InvalidJSON) {
    std::string invalidMessage = "{invalid json format}";

    // El listener debe manejar el error sin crashear
    EXPECT_THROW({
        nlohmann::json json = nlohmann::json::parse(invalidMessage);
    }, nlohmann::json::parse_error);
}

// Test: Validar que un JSON sin campos requeridos se maneje correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_MissingFields) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    // Falta matchId, homeTeamScore, visitorTeamScore

    std::string message = messageJson.dump();

    // Debe lanzar excepción al intentar deserializar
    EXPECT_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();
    }, nlohmann::json::exception);
}

// Test: Validar que un JSON con campo tournamentId faltante se maneje correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_MissingTournamentId) {
    nlohmann::json messageJson;
    messageJson["matchId"] = "match-123";
    messageJson["homeTeamScore"] = 3;
    messageJson["visitorTeamScore"] = 2;

    std::string message = messageJson.dump();

    EXPECT_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();
    }, nlohmann::json::exception);
}

// Test: Validar que un JSON con campo matchId faltante se maneje correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_MissingMatchId) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["homeTeamScore"] = 3;
    messageJson["visitorTeamScore"] = 2;

    std::string message = messageJson.dump();

    EXPECT_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();
    }, nlohmann::json::exception);
}

// Test: Validar que un JSON con tipos incorrectos se maneje correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_IncorrectTypes) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-123";
    messageJson["homeTeamScore"] = "three"; // String en lugar de int
    messageJson["visitorTeamScore"] = 2;

    std::string message = messageJson.dump();

    EXPECT_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();
    }, nlohmann::json::exception);
}

// ============================================================================
// Tests de contenido del mensaje parseado
// ============================================================================

// Test: Validar que todos los campos se parsean correctamente
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_AllFieldsParsedCorrectly) {
    std::string tournamentId = "550e8400-e29b-41d4-a716-446655440000";
    std::string matchId = "match-abc-123";
    int homeScore = 7;
    int visitorScore = 5;

    nlohmann::json messageJson;
    messageJson["tournamentId"] = tournamentId;
    messageJson["matchId"] = matchId;
    messageJson["homeTeamScore"] = homeScore;
    messageJson["visitorTeamScore"] = visitorScore;

    std::string message = messageJson.dump();

    nlohmann::json json = nlohmann::json::parse(message);
    domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

    EXPECT_EQ(event.tournamentId, tournamentId);
    EXPECT_EQ(event.matchId, matchId);
    EXPECT_EQ(event.homeTeamScore, homeScore);
    EXPECT_EQ(event.visitorTeamScore, visitorScore);
}

// Test: Validar que se pueda parsear un mensaje con IDs largos
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_LongIds) {
    std::string longTournamentId = "550e8400-e29b-41d4-a716-446655440000-very-long-id-extension";
    std::string longMatchId = "match-550e8400-e29b-41d4-a716-446655440000-extension";

    nlohmann::json messageJson;
    messageJson["tournamentId"] = longTournamentId;
    messageJson["matchId"] = longMatchId;
    messageJson["homeTeamScore"] = 1;
    messageJson["visitorTeamScore"] = 1;

    std::string message = messageJson.dump();

    nlohmann::json json = nlohmann::json::parse(message);
    domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

    EXPECT_EQ(event.tournamentId, longTournamentId);
    EXPECT_EQ(event.matchId, longMatchId);
}

// ============================================================================
// Tests de validación de datos de negocio
// ============================================================================

// Test: Los scores negativos se parsean (la validación se hace en la lógica de negocio)
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_NegativeScores_Parsed) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-123";
    messageJson["homeTeamScore"] = -1;
    messageJson["visitorTeamScore"] = -2;

    std::string message = messageJson.dump();

    // El listener debe poder parsear scores negativos
    // La validación de negocio (rechazar negativos) se hace en el Delegate
    EXPECT_NO_THROW({
        nlohmann::json json = nlohmann::json::parse(message);
        domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

        EXPECT_EQ(event.homeTeamScore, -1);
        EXPECT_EQ(event.visitorTeamScore, -2);
    });
}

// Test: Validar empate (scores iguales)
TEST_F(MatchScoreUpdateListenerTest, ProcessMessage_TieScore) {
    nlohmann::json messageJson;
    messageJson["tournamentId"] = "550e8400-e29b-41d4-a716-446655440000";
    messageJson["matchId"] = "match-123";
    messageJson["homeTeamScore"] = 5;
    messageJson["visitorTeamScore"] = 5;

    std::string message = messageJson.dump();

    nlohmann::json json = nlohmann::json::parse(message);
    domain::ScoreUpdateEvent event = json.get<domain::ScoreUpdateEvent>();

    EXPECT_EQ(event.homeTeamScore, event.visitorTeamScore);
}
