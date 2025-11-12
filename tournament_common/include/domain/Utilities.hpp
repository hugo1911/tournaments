#ifndef DOMAIN_UTILITIES_HPP
#define DOMAIN_UTILITIES_HPP

#include <nlohmann/json.hpp>
#include "domain/Team.hpp"
#include "domain/Tournament.hpp"
#include "domain/Group.hpp"
#include "domain/Match.hpp"

namespace domain {

    inline void to_json(nlohmann::json& json, const Team& team) {
        json = {{"id", team.Id}, {"name", team.Name}};
    }

    inline void from_json(const nlohmann::json& json, Team& team) {
        if(json.contains("id")) {
            json.at("id").get_to(team.Id);
        }
        json.at("name").get_to(team.Name);
    }

    inline void from_json(const nlohmann::json& json, std::vector<Team>& teams) {
        for (auto j = json.begin(); j != json.end(); ++j) {
            Team team;
            if(j.value().contains("id")) {
                j.value().at("id").get_to(team.Id);
            }
            if(j.value().contains("name")) {
                j.value().at("name").get_to(team.Name);
            }
            teams.push_back(team);
        }
    }

    inline void to_json(nlohmann::json& json, const std::shared_ptr<Team>& team) {
        json = nlohmann::basic_json();
        json["name"] = team->Name;

        if (!team->Id.empty()) {
            json["id"] = team->Id;
        }
    }

    inline TournamentType fromString(std::string_view type) {
        if (type == "DOUBLE_ELIMINATION")
            return TournamentType::DOUBLE_ELIMINATION;

        return TournamentType::DOUBLE_ELIMINATION;
    }

    inline void from_json(const nlohmann::json& json, TournamentFormat& format) {
        if(json.contains("maxTeamsPerGroup"))
            json.at("maxTeamsPerGroup").get_to(format.MaxTeamsPerGroup());
        if(json.contains("numberOfGroups"))
            json.at("numberOfGroups").get_to(format.NumberOfGroups());
        if(json.contains("type"))
            format.Type() = fromString(json["type"].get<std::string>());
    }

    inline void to_json(nlohmann::json& json, const TournamentFormat& format) {
        json = {{"maxTeamsPerGroup", format.MaxTeamsPerGroup()}, {"numberOfGroups", format.NumberOfGroups()}};
        switch (format.Type()) {
            case TournamentType::DOUBLE_ELIMINATION:
                json["type"] = "DOUBLE_ELIMINATION";
                break;
            default:
                json["type"] = "DOUBLE_ELIMINATION";
        }
    }

    inline void to_json(nlohmann::json& json, const std::shared_ptr<Tournament>& tournament) {
        json = {{"name", tournament->Name()}};
        if (!tournament->Id().empty()) {
            json["id"] = tournament->Id();
        }
        json["format"] = tournament->Format();
    }

    inline void from_json(const nlohmann::json& json, std::shared_ptr<Tournament>& tournament) {
        if(json.contains("id")) {
            tournament->Id() = json["id"].get<std::string>();
        }
        json["name"].get_to(tournament->Name());
        if (json.contains("format"))
            json.at("format").get_to(tournament->Format());
    }

    inline void to_json(nlohmann::json& json, const Tournament& tournament) {
        json = {{"name", tournament.Name()}};
        if (!tournament.Id().empty()) {
            json["id"] = tournament.Id();
        }
        json["format"] = tournament.Format();
    }

    inline void from_json(const nlohmann::json& json, Tournament& tournament) {
        if(json.contains("id")) {
            tournament.Id() = json["id"].get<std::string>();
        }
        json["name"].get_to(tournament.Name());
        if (json.contains("format"))
            json.at("format").get_to(tournament.Format());
    }

    inline void from_json(const nlohmann::json& json, Group& group) {
        if(json.contains("id")) {
            group.Id() = json["id"].get<std::string>();
        }
        if(json.contains("tournamentId")) {
            group.TournamentId() = json["tournamentId"].get<std::string>();
        }
        json["name"].get_to(group.Name());
        if(json.contains("teams") && json["teams"].is_array()) {
            json["teams"].get_to(group.Teams());
        }
    }

    inline void to_json(nlohmann::json& json, const std::shared_ptr<Group>& group) {
        json["name"] = group->Name();
        json["tournamentId"] = group->TournamentId();
        if (!group->Id().empty()) {
            json["id"] = group->Id();
        }
        json["teams"] = group->Teams();
    }

    inline void to_json(nlohmann::json& json, const std::vector<std::shared_ptr<Group>>& groups) {
        json = nlohmann::json::array();
        for (const auto& group : groups) {
            auto jsonGroup = nlohmann::json();
            jsonGroup["name"] = group->Name();
            jsonGroup["tournamentId"] = group->TournamentId();
            if (!group->Id().empty()) {
                jsonGroup["id"] = group->Id();
            }
            jsonGroup["teams"] = group->Teams();
            json.push_back(jsonGroup);
        }
    }

    inline void to_json(nlohmann::json& json, const Group& group) {
        json["name"] = group.Name();
        json["tournamentId"] = group.TournamentId();
        if (!group.Id().empty()) {
            json["id"] = group.Id();
        }
        json["teams"] = group.Teams();
    }

    // inline std::string bracketTypeToString(BracketType type) {
    //     switch (type) {
    //         case BracketType::WINNERS: return "WINNERS";
    //         case BracketType::LOSERS: return "LOSERS";
    //         case BracketType::FINAL: return "FINAL";
    //         default: return "WINNERS";
    //     }
    // }

    // inline BracketType bracketTypeFromString(std::string_view type) {
    //     if (type == "WINNERS") return BracketType::WINNERS;
    //     if (type == "LOSERS") return BracketType::LOSERS;
    //     if (type == "FINAL") return BracketType::FINAL;
    //     return BracketType::WINNERS;
    // }

    inline void to_json(nlohmann::json& json, const Score& score) {
        json = {
            {"home", score.homeTeamScore},
            {"visitor", score.visitorTeamScore}
        };
    }

    inline void from_json(const nlohmann::json& json, Score& score) {
        if (json.contains("home"))
            json.at("home").get_to(score.homeTeamScore);
        else if (json.contains("homeTeamScore"))
            json.at("homeTeamScore").get_to(score.homeTeamScore);

        if (json.contains("visitor"))
            json.at("visitor").get_to(score.visitorTeamScore);
        else if (json.contains("visitorTeamScore"))
            json.at("visitorTeamScore").get_to(score.visitorTeamScore);
    }

    inline void to_json(nlohmann::json& json, const Match& match) {
        json = nlohmann::json::object();

        // Home team object
        if (!match.HomeTeamId().empty()) {
            json["home"] = {
                {"id", match.HomeTeamId()}
            };
            if (!match.HomeTeamName().empty()) {
                json["home"]["name"] = match.HomeTeamName();
            }
        }

        // Visitor team object
        if (!match.VisitorTeamId().empty()) {
            json["visitor"] = {
                {"id", match.VisitorTeamId()}
            };
            if (!match.VisitorTeamName().empty()) {
                json["visitor"]["name"] = match.VisitorTeamName();
            }
        }

        // Round
        if (!match.Round().empty()) {
            json["round"] = match.Round();
        } else if (!match.Name().empty()) {
            // Fallback to name if round is not set
            json["round"] = match.Name();
        }

        // Score
        json["score"] = match.MatchScore();
    }

    inline void from_json(const nlohmann::json& json, Match& match) {
        if (json.contains("id")) {
            match.Id() = json["id"].get<std::string>();
        }
        if (json.contains("name")) {
            match.Name() = json["name"].get<std::string>();
        }
        if (json.contains("round")) {
            match.Round() = json["round"].get<std::string>();
        }
        if (json.contains("tournamentId")) {
            match.TournamentId() = json["tournamentId"].get<std::string>();
        }

        // Parse home team
        if (json.contains("home") && json["home"].is_object()) {
            if (json["home"].contains("id")) {
                match.HomeTeamId() = json["home"]["id"].get<std::string>();
            }
            if (json["home"].contains("name")) {
                match.HomeTeamName() = json["home"]["name"].get<std::string>();
            }
        } else if (json.contains("homeTeamId")) {
            match.HomeTeamId() = json["homeTeamId"].get<std::string>();
        }

        // Parse visitor team
        if (json.contains("visitor") && json["visitor"].is_object()) {
            if (json["visitor"].contains("id")) {
                match.VisitorTeamId() = json["visitor"]["id"].get<std::string>();
            }
            if (json["visitor"].contains("name")) {
                match.VisitorTeamName() = json["visitor"]["name"].get<std::string>();
            }
        } else if (json.contains("visitorTeamId")) {
            match.VisitorTeamId() = json["visitorTeamId"].get<std::string>();
        }

        if (json.contains("score")) {
            json.at("score").get_to(match.MatchScore());
        } else if (json.contains("matchScore")) {
            json.at("matchScore").get_to(match.MatchScore());
        }
    }

    inline void to_json(nlohmann::json& json, const std::shared_ptr<Match>& match) {
        json = nlohmann::json::object();

        // Home team object
        if (!match->HomeTeamId().empty()) {
            json["home"] = {
                {"id", match->HomeTeamId()}
            };
            if (!match->HomeTeamName().empty()) {
                json["home"]["name"] = match->HomeTeamName();
            }
        }

        // Visitor team object
        if (!match->VisitorTeamId().empty()) {
            json["visitor"] = {
                {"id", match->VisitorTeamId()}
            };
            if (!match->VisitorTeamName().empty()) {
                json["visitor"]["name"] = match->VisitorTeamName();
            }
        }

        // Round
        if (!match->Round().empty()) {
            json["round"] = match->Round();
        } else if (!match->Name().empty()) {
            // Fallback to name if round is not set
            json["round"] = match->Name();
        }

        // Score
        json["score"] = match->MatchScore();
    }

    inline void to_json(nlohmann::json& json, const std::vector<std::shared_ptr<Match>>& matches) {
        json = nlohmann::json::array();
        for (const auto& match : matches) {
            nlohmann::json matchJson;
            to_json(matchJson, match);
            json.push_back(matchJson);
        }
    }
}

#endif /* FC7CD637_41CC_48DE_8D8A_BC2CFC528D72 */
