// F1 TERMINAL RACER 2025 - ENHANCED EDITION
// Compile with: g++ -std=c++17 f1_enhanced.cpp -o f1

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <limits>
#include <sstream>
#include <windows.h>
#include <map>

using namespace std;

// ---------- Utility & Globals ----------
#ifdef _WIN32
const char *CLEAR_CMD = "cls";
#else
const char *CLEAR_CMD = "clear";

#endif

std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

void pressAnyKey()
{
    cout << "\nPress Enter to continue . . .";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

string formatTime(double seconds)
{
    if (seconds < 0.0)
        seconds = 0.0;
    int total_ms = (int)round(seconds * 1000.0);
    int ms = total_ms % 1000;
    int total_s = total_ms / 1000;
    int s = total_s % 60;
    int m = total_s / 60;
    char buf[64];
    sprintf(buf, "%d:%02d.%03d", m, s, ms);
    return string(buf);
}

template <typename T>
T clampVal(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

// ---------- Data Structures ----------

struct Driver
{
    string name;
    int speed, cornering, overtaking, consistency, aggression, strategy;
    string team;
};

struct Track
{
    string name, country, asciiMap;
    double baseLapSec;
    int difficulty, corners;
    double pitStopTime;
};

struct Racer
{
    string displayName;
    Driver driver;
    double cumulativeTime = 0.0, lastLapTime = 0.0, fastestLap = 1e9;
    double tyre = 100.0, vehicle = 100.0;
    int startingPos = 0, currentPos = 0, pitStops = 0;
    bool inPitThisLap = false;
};

struct Team
{
    string name;
    double performance;
    string carModel;
    int budget;
};

// ---------- Game Content ----------

map<string, Team> teams = {
    {"Red Bull", {"Red Bull Racing", 9.8, "RB21", 185000000}},
    {"Ferrari", {"Scuderia Ferrari", 9.2, "SF-25", 145000000}},
    {"Mercedes", {"Mercedes-AMG", 9.0, "W16", 155000000}},
    {"McLaren", {"McLaren", 8.7, "MCL38", 135000000}},
    {"Aston Martin", {"Aston Martin", 8.5, "AMR24", 125000000}},
    {"Alpine", {"Alpine", 7.8, "A524", 95000000}},
    {"RB", {"RB", 7.5, "VCARB01", 85000000}},
    {"Haas", {"Haas", 7.2, "VF-24", 80000000}},
    {"Williams", {"Williams", 7.0, "FW46", 75000000}},
    {"Sauber", {"Kick Sauber", 6.8, "C44", 70000000}}};

map<string, vector<Driver>> teamDrivers = {
    {"Red Bull", {{"Max Verstappen", 10, 10, 10, 10, 9, 9, "Red Bull"}, {"Sergio Perez", 8, 7, 8, 8, 7, 8, "Red Bull"}}},
    {"Ferrari", {{"Charles Leclerc", 9, 9, 8, 8, 8, 8, "Ferrari"}, {"Carlos Sainz", 8, 8, 7, 9, 7, 8, "Ferrari"}}},
    {"Mercedes", {{"Lewis Hamilton", 9, 8, 8, 9, 7, 9, "Mercedes"}, {"George Russell", 8, 8, 8, 8, 8, 8, "Mercedes"}}},
    {"McLaren", {{"Lando Norris", 9, 9, 9, 9, 8, 8, "McLaren"}, {"Oscar Piastri", 8, 8, 8, 8, 7, 7, "McLaren"}}},
    {"Aston Martin", {{"Fernando Alonso", 8, 8, 8, 9, 7, 9, "Aston Martin"}, {"Lance Stroll", 6, 7, 7, 6, 7, 6, "Aston Martin"}}},
    {"Alpine", {{"Pierre Gasly", 7, 7, 7, 7, 8, 7, "Alpine"}, {"Esteban Ocon", 7, 7, 7, 8, 7, 7, "Alpine"}}},
    {"RB", {{"Yuki Tsunoda", 8, 6, 7, 7, 8, 6, "RB"}, {"Daniel Ricciardo", 7, 7, 8, 7, 7, 7, "RB"}}},
    {"Haas", {{"Nico Hulkenberg", 7, 8, 7, 8, 6, 8, "Haas"}, {"Kevin Magnussen", 7, 6, 7, 6, 8, 6, "Haas"}}},
    {"Williams", {{"Alex Albon", 7, 7, 8, 7, 7, 7, "Williams"}, {"Logan Sargeant", 6, 6, 6, 6, 6, 6, "Williams"}}},
    {"Sauber", {{"Valtteri Bottas", 7, 7, 6, 8, 6, 8, "Sauber"}, {"Zhou Guanyu", 6, 7, 6, 7, 6, 7, "Sauber"}}}};

map<string, Track> tracks = {
    {"Monaco", {
        "Monaco Street Circuit", 
        "Monaco", 
        "│               _____                 │\n"
        "│              /     \\                │\n"
        "│   __________/       │               │\n"
        "│  │                  │               │\n"
        "│  │                  │               │\n" 
        "│  │         ________/                │\n"
        "│  │        │                         │\n"
        "│  │         \\                        │\n"
        "│   \\         \\                       │\n"
        "│    \\_________│                      │",
        78.5,  // lapDistance (km)
        9,     // laps
        11,    // corners
        17.0   // pitstopTime
    }},
    
    {"Spa", {
        "Spa-Francorchamps", 
        "Belgium", 
        "│   _______                           │\n"
        "│  /       \\_________                 │\n"
        "│ /                   \\               │\n"
        "│ |                    |              │\n"
        "│ |                    |              │\n"
        "│  \\                  /               │\n"
        "│   \\________ _______/                │\n"
        "│           \\_/                       │", 
        99.0,  // lapDistance (km)
        8,     // laps
        14,    // corners
        21.0   // pitstopTime
    }},
    
    {"Silverstone", {
        "Silverstone Circuit", 
        "UK", 
        "│    ____                             │\n"
        "│   /    \\          ____              │\n"
        "│  /      \\____    |    |             │\n"
        "│ |      ______|   /    |             │\n"
        "│ |     |_________|     |             │\n"
        "│  \\                   /              │\n"
        "│   \\_________________/               │", 
        95.0,  // lapDistance (km)
        6,     // laps
        17,    // corners
        20.0   // pitstopTime
    }},
    
    {"Monza", {
        "Autodromo Nazionale Monza", 
        "Italy", 
        "│   ____________________              │\n"
        "│  |                    |             │\n"
        "│  |                    |             │\n"
        "│  |    __        __    |             │\n"
        "│  |   |  |      |  |   |             │\n"
        "│  |   |  |      |  |   |             │\n"
        "│   \\__|  |______|  |__/              │", 
        85.0,  // lapDistance (km)
        5,     // laps
        14,    // corners
        24.0   // pitstopTime
    }}
};

// ---------- Core Logic ----------

double driverSkillIndex(const Driver &d)
{
    return (d.speed + d.cornering + d.overtaking + d.consistency + d.aggression + d.strategy) / 6.0;
}

double computeLapTimeSeconds(const Racer &racer, const Track &track, int mode, bool isPlayer)
{
    double base = track.baseLapSec;
    double skill = driverSkillIndex(racer.driver);
    double skillReduction = (skill - 7.0) * 0.6;

    double tyreFactor = 1.0;
    if (racer.tyre < 80.0)
        tyreFactor += (80.0 - racer.tyre) * 0.0018;
    if (racer.tyre < 60.0)
        tyreFactor += 0.01;
    if (racer.tyre < 40.0)
        tyreFactor += 0.02;

    double vehicleFactor = 1.0 + (100.0 - racer.vehicle) * 0.001;
    double modeDelta = (mode == 1) ? -0.6 : ((mode == 0) ? 0.4 : 0.0);

    uniform_real_distribution<double> jitterDist(-0.6, 0.6);
    double jitter = jitterDist(rng);

    double lap = base + skillReduction + modeDelta;
    lap *= tyreFactor * vehicleFactor;
    lap += jitter;

    return max(lap, 30.0);
}

void applyWearAndDamage(Racer &racer, int mode, bool hadPitThisLap)
{
    if (hadPitThisLap)
    {
        racer.tyre = 100.0;
        racer.vehicle = clampVal(racer.vehicle + 2.0, 0.0, 100.0);
        return;
    }

    double tyreDrop = 2.5;
    double vehicleDrop = 0.0;

    if (mode == 1)
    {
        tyreDrop = 5.0 + uniform_real_distribution<double>(-0.5, 1.5)(rng);
        vehicleDrop = 0.8;
    }
    else if (mode == 0)
    {
        tyreDrop = 1.8 + uniform_real_distribution<double>(-0.4, 0.6)(rng);
        vehicleDrop = 0.2;
    }

    racer.tyre = clampVal(racer.tyre - tyreDrop, 0.0, 100.0);
    racer.vehicle = clampVal(racer.vehicle - vehicleDrop, 0.0, 100.0);
}

vector<Racer> makeField(const Driver &playerDrv, const string &playerName)
{
    vector<Racer> field;

    // Create player

    Racer player;
    player.displayName = playerName;
    player.driver = playerDrv;
    field.push_back(player);

    // Create AI opponents from all teams

    for (auto &team : teamDrivers)
    {
        for (auto &driver : team.second)
        {
            if (driver.name != playerName)
            {
                Racer ai;
                ai.displayName = driver.name;
                ai.driver = driver;
                field.push_back(ai);
            }
        }
    }

    return field;
}

void recomputePositions(vector<Racer> &field)
{
    vector<int> idx(field.size());
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b)
         { return field[a].cumulativeTime < field[b].cumulativeTime; });

    for (int i = 0; i < (int)idx.size(); ++i)
    {
        field[idx[i]].currentPos = i + 1;
    }
}

int aiChooseStrategy(const Racer &r)
{
    if (r.tyre < 35.0)
        return 2;
    double skill = driverSkillIndex(r.driver);
    double pushChance = 0.25 + (skill - 7.0) * 0.08;
    return (uniform_real_distribution<double>(0.0, 1.0)(rng) < pushChance) ? 1 : 0;
}

string generateCommentary(const Racer &player, int oldPos, int newPos, double tyre, bool pitThisLap, int lap, int totalLaps, double lastLapTime)
{
    vector<string> commentaries;

    // Pit stop commentary

    if (pitThisLap)
    {
        vector<string> pitComments = {
            "Box box box! Wait, that's actually our driver!",
            "Pit crew woke up! Changing tires in record time!",
            "Fresh rubber! Now let's hope they remember to remove the tire warmers!",
            "Strategic masterstroke... or desperate gamble? Time will tell!",
            "Pit stop! The crew moves like they've had one too many energy drinks!"};
        return pitComments[uniform_int_distribution<int>(0, pitComments.size() - 1)(rng)];
    }

    // Position changes

    if (newPos < oldPos)
    {
        vector<string> overtakeComments = {
            "THROUGH GOES THE CAR! What a move! The crowd goes wild!",
            "Overtake! That was cleaner than my browser history!",
            "Bold move! The other driver is checking his mirrors in shame!",
            "Like a hot knife through butter! Beautiful pass!",
            "Send him an invoice for that overtake - pure robbery!"};
        return overtakeComments[uniform_int_distribution<int>(0, overtakeComments.size() - 1)(rng)];
    }

    if (newPos > oldPos)
    {
        vector<string> lostPosComments = {
            "Ouch! Lost a position. Defense was about as solid as wet paper!",
            "Got overtaken! Time to activate the secret DRS... oh wait, we're the one being passed!",
            "Position lost! The engineer is facepalming right now!",
            "Well, that didn't go according to plan! Got mugged on the straight!",
            "Defense? What defense? We're handing out positions like free samples!"};
        return lostPosComments[uniform_int_distribution<int>(0, lostPosComments.size() - 1)(rng)];
    }

    // Tire warnings

    if (tyre < 30.0)
    {
        vector<string> tyreComments = {
            "Tires are deader than my social life! BOX NOW!",
            "Rubber? What rubber? I see only smoke and prayers!",
            "Tires crying louder than my bank account! Pit window is WIDE open!",
            "If tires could talk, they'd be screaming for retirement!",
            "Tire degradation so bad, we're basically on rims!"};
        return tyreComments[uniform_int_distribution<int>(0, tyreComments.size() - 1)(rng)];
    }

    if (tyre < 50.0)
    {
        vector<string> tyreWarnComments = {
            "Tires starting to complain louder than my stomach before lunch!",
            "Rubber is getting spicy! Might want to think about a pit stop soon!",
            "Tires have more graining than a farmer's field!",
            "The tires are asking for a pension plan! Still some life left though!",
            "Tire wear: 'Send Help' - signed, your Pirellis"};
        return tyreWarnComments[uniform_int_distribution<int>(0, tyreWarnComments.size() - 1)(rng)];
    }

    // Fast lap commentary

    if (lastLapTime < 85.0)
    {
        vector<string> fastComments = {
            "PURPLE SECTOR! That lap was quicker than my WiFi connection!",
            "New fastest lap! The car is flying like it stole something!",
            "That lap was so fast, it probably broke the space-time continuum!",
            "Quickest lap of the race! Engineers are high-fiving in the garage!",
            "That lap was cleaner than my room when mom visits! Absolutely flying!"};
        return fastComments[uniform_int_distribution<int>(0, fastComments.size() - 1)(rng)];
    }

    // Lap-specific funny commentary

    if (lap == 1)
    {
        vector<string> lap1Comments = {
            "Lights out and away we go! Wait, we already started?",
            "First lap chaos! Everyone fighting like it's Black Friday!",
            "The race begins! So much action, my head is spinning faster than the tires!",
            "Lap 1: Where talent meets pure chaos!",
            "And we're racing! Cars everywhere like ants at a picnic!"};
        return lap1Comments[uniform_int_distribution<int>(0, lap1Comments.size() - 1)(rng)];
    }

    if (lap == totalLaps)
    {
        vector<string> finalComments = {
            "Final lap! Give it everything! The champagne is getting warm!",
            "Last lap! Push like your Instagram depends on it!",
            "Final tour! Time to empty the tank... metaphorically of course!",
            "One lap to go! The checkered flag is getting lonely!",
            "Last lap! Driving like there's free pizza at the finish line!"};
        return finalComments[uniform_int_distribution<int>(0, finalComments.size() - 1)(rng)];
    }

    // Random funny commentary for normal laps

    vector<string> randomComments = {
        "Car sounds happier than my dog with a new toy! Good pace!",
        "Smooth operator! Driving like they're on a Sunday cruise!",
        "Consistent laps! More reliable than my alarm clock!",
        "Managing the gap like a pro! The others are just spectators now!",
        "This driver has more rhythm than my Spotify playlist!",
        "Pace is solid! The car looks planted... unlike my hair in humidity!",
        "Good sector times! Engineers are probably taking a coffee break!",
        "Consistency is key! Driving like they've done this before!",
        "The gap is stable! Other drivers seeing nothing but exhaust fumes!",
        "Beautiful driving! Making it look easier than breathing!",
        "Car handling like a dream! If only my love life was this smooth!",
        "Pace is strong! The competition is eating our dust!",
        "Driving with the confidence of someone who found the last parking spot!",
        "Lap times so consistent, they're boring the statisticians!",
        "The car is dancing through corners like it's Saturday night!"};

    // Different commentary based on lap number for variety

    int commentIndex = (lap + (int)lastLapTime) % randomComments.size();
    return randomComments[commentIndex];
}

// ---------- UI Functions ----------

void clearScreen() { system(CLEAR_CMD); }

void showAbout()
{
    clearScreen();
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│    🏎️  F1 TERMINAL RACER 2025  🏎️     │\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│                                     │\n";
    cout << "│   ██████ ██    ████████  ████████   │\n";
    cout << "|   ██     ██    ██    ██  ██         │\n";
    cout << "│   ██████ ██    ████████  ████████   │\n";
    cout << "│   ██     ██    ██    ██        ██   │\n";
    cout << "│   ██     ██    ██    ██  ████████   │\n";
    cout << "│                                     │\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│  🏆 CORE FEATURES                   │\n";
    cout << "│  ┌─────────────────────────────┐    │\n";
    cout << "│  │ • 10 Authentic F1 Teams     │    │\n";
    cout << "│  │ • 20 Real Driver Lineup     │    │\n";
    cout << "│  │ • 4 Legendary Circuits      │    │\n";
    cout << "│  │ • Live Strategy Decisions   │    │\n";
    cout << "│  └─────────────────────────────┘    │\n";
    cout << "│                                     │\n";
    cout << "│  🎯 RACING EXPERIENCE               │\n";
    cout << "│  ┌─────────────────────────────┐    │\n";
    cout << "│  │ • Real Tire Degradation     │    │\n";
    cout << "│  │ • Track-Specific Pit Stops  │    │\n";
    cout << "│  │ • Push/Save/Pit Strategy    │    │\n";
    cout << "│  │ • Live Gap Tracking         │    │\n";
    cout << "│  └─────────────────────────────┘    │\n";
    cout << "│                                     │\n";
    cout << "│  🎮 GAMEPLAY HIGHLIGHTS             │\n";
    cout << "│  ┌─────────────────────────────┐    │\n";
    cout << "│  │ • Dynamic Engineer Radio    │    │\n";
    cout << "│  │ • Position Battles          │    │\n";
    cout << "│  │ • Fastest Lap Competition   │    │\n";
    cout << "│  │ • Professional UI Design    │    │\n";
    cout << "│  └─────────────────────────────┘    │\n";
    cout << "│                                     │\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│  Developed with 💙 by Aniruddh      │\n";
    cout << "│  Version 1.0 · © 2025               │\n";
    cout << "│                                     │\n";
    cout << "│  \"To finish first, first you must   │\n";
    cout << "│    finish.\" - Enzo Ferrari          │\n";
    cout << "└─────────────────────────────────────┘\n";

    cout << "\nPress Enter for developer message...";
    pressAnyKey();

    clearScreen();
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│      FROM THE DEVELOPER 🛠️           │\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│                                     │\n";
    cout << "│  Thank you for playing F1 Terminal  │\n";
    cout << "│  Racer! This project combines my    │\n";
    cout << "│  passion for Formula 1 racing with  │\n";
    cout << "│  creative coding. Every lap time,   │\n";
    cout << "│  tire strategy, and overtake is     │\n";
    cout << "│  calculated to bring you the most   │\n";
    cout << "│  authentic text-based F1 experience!│\n";
    cout << "│                                     │\n";
    cout << "│  Remember:                          │\n";
    cout << "│  • Push hard when tires are fresh   │\n";
    cout << "│  • Pit when tires drop below 40%    │\n";
    cout << "│  • Watch for engineer hints!        │\n";
    cout << "│                                     │\n";
    cout << "│  Enjoy the race! 🏁                 │\n";
    cout << "│                                     │\n";
    cout << "└─────────────────────────────────────┘\n";
    pressAnyKey();
}

string getTeamSelection()
{
    clearScreen();
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│          CHOOSE YOUR TEAM           │\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│                                     │\n";

    vector<string> teamNames;
    for (auto &team : teams)
        teamNames.push_back(team.first);

    for (int i = 0; i < 10; ++i)
    {
        string icon = "🟠";
        if (i == 1)
            icon = "🔴";
        else if (i == 2)
            icon = "⚫";
        else if (i == 3)
            icon = "🟠";
        else if (i == 4)
            icon = "🟢";
        else if (i == 5)
            icon = "🔵";
        else if (i == 6)
            icon = "🐮";
        else if (i == 7)
            icon = "⚪";
        else if (i == 8)
            icon = "🔵";
        else if (i == 9)
            icon = "🟤";

        printf("│   %s %2d. %-16s ⭐%.1f     │\n",
               icon.c_str(), i + 1, teamNames[i].c_str(), teams[teamNames[i]].performance);
    }
    cout << "│   0. GO BACK                        │\n";
    cout << "│                                     │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Enter choice (0-10): ";

    string input;
    getline(cin, input);
    int choice = clampVal(stoi(input), 0, 10);
    if (choice == 0)
        return "";
    return teamNames[choice - 1];
}

Driver getDriverSelection(const string &team)
{
    clearScreen();
    cout << "┌───────────────────────────────────────┐\n";
    cout << "│        " << team << " - " << teams[team].carModel;
    for (int i = 0; i < 20 - team.length() - teams[team].carModel.length(); ++i)
        cout << " ";
    cout << "        │\n";
    cout << "├───────────────────────────────────────┤\n";
    printf("│ Team Principal: %-19s   │\n", (team == "Ferrari") ? "Frédéric Vasseur" : "Toto Wolff");
    printf("│ Budget: $%dM | Performance: %.1f      │\n", teams[team].budget / 1000000, teams[team].performance);
    cout << "├───────────────────────────────────────┤\n";
    cout << "│                                       │\n";

    auto drivers = teamDrivers[team];
    for (int i = 0; i < (int)drivers.size(); ++i)
    {
        string driverIcon = (team == "Ferrari") ? "🟥" : "🟠";
        printf("│   %s %-18s ⭐%.1f         │\n",
               driverIcon.c_str(), drivers[i].name.c_str(), driverSkillIndex(drivers[i]));
        cout << "│   ┌───────────────────────────┐       │\n";
        printf("│   │ Speed: %-2d  Corner: %-2d     │       │\n", drivers[i].speed, drivers[i].cornering);
        printf("│   │ Overtake: %-2d  Consist:%-2d  │       │\n", drivers[i].overtaking, drivers[i].consistency);
        cout << "│   └───────────────────────────┘       │\n";
        if (i < (int)drivers.size() - 1)
            cout << "│                                       │\n";
    }

    cout << "│                                       │\n";
    cout << "├───────────────────────────────────────┤\n";
    cout << "│   0. GO BACK                          │\n";
    cout << "│                                       │\n";
    cout << "└───────────────────────────────────────┘\n";
    cout << "Choose driver (0-" << drivers.size() << "): ";

    string input;
    getline(cin, input);
    int choice = clampVal(stoi(input), 0, (int)drivers.size());
    if (choice == 0)
    {
        Driver emptyDriver;
        return emptyDriver;
    }
    return drivers[choice - 1];
}

string getTrackSelection()
{
    clearScreen();
    cout << "┌──────────────────────────────────────┐\n";
    cout << "│          CHOOSE CIRCUIT              │\n";
    cout << "├──────────────────────────────────────┤\n";
    cout << "│                                      │\n";

    vector<string> trackNames;
    for (auto &track : tracks)
        trackNames.push_back(track.first);

    for (int i = 0; i < (int)trackNames.size(); ++i)
    {
        string difficulty;
        if (tracks[trackNames[i]].difficulty >= 8)
            difficulty = "HARD 🏔️      ";
        else if (tracks[trackNames[i]].difficulty >= 6)
            difficulty = "MEDIUM ⚖️    ";
        else
            difficulty = "EASY 🌟";

        printf("│   %d. %-16s                │\n", i + 1, trackNames[i].c_str());
        printf("│      Length: %.2fkm | Laps: 25       │\n", tracks[trackNames[i]].baseLapSec / 10.0);
        printf("│      Pit Stop: %.1fs                 │\n", tracks[trackNames[i]].pitStopTime);
        printf("│      Difficulty:%-14s         │\n", difficulty.c_str());
        if (i < (int)trackNames.size() - 1)
            cout << "│                                      │\n";
    }

    cout << "│                                      │\n";
    cout << "├──────────────────────────────────────┤\n";
    cout << "│   0. GO BACK                         │\n";
    cout << "│                                      │\n";
    cout << "└──────────────────────────────────────┘\n";
    cout << "Enter choice (0-" << trackNames.size() << "): ";

    string input;
    getline(cin, input);
    int choice = clampVal(stoi(input), 0, (int)trackNames.size());
    if (choice == 0)
        return "";
    
    string selectedTrack = trackNames[choice - 1];

    // SHOW TRACK MAP AFTER SELECTION
    clearScreen();
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│       SELECTED: " << selectedTrack;
    for (int i = 0; i < 20 - selectedTrack.length(); i++) cout << " ";
    cout << "│\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│ " << tracks[selectedTrack].name << "               │"<<endl;
    cout << "│ " << tracks[selectedTrack].country << "                              │"<<endl;
    cout << "│                                     │\n";
    
    // Display track map
    stringstream ss(tracks[selectedTrack].asciiMap);
    string line;
    while (getline(ss, line)) {
        cout << line << endl;
    }
    
    cout << "│                                     │\n";
    cout << "│ Track Length: " << tracks[selectedTrack].baseLapSec / 10.0 << "km" <<"                │"<< endl;
    cout << "│ Laps: 25 | Corners: " << tracks[selectedTrack].corners <<"              │" << endl;
    cout << "│ Difficulty: " << tracks[selectedTrack].difficulty << "/10" "                    │" << endl;
    cout << "│ Pit Stop Time: " << tracks[selectedTrack].pitStopTime << "s" "                  │"<< endl;
    cout << "│                                     │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Press Enter to continue to race...";
    getline(cin, input);

    return selectedTrack;
}

// Engineer advice

string getEngineerAdvice(const Racer &player, int lap, int totalLaps, int playerPos, const vector<Racer> &field)
{
    vector<string> advice;

    // Tire advice
    if (player.tyre < 40.0 && lap < totalLaps - 5)
    {
        advice.push_back("Tires at " + to_string((int)player.tyre) + "% - consider pitting soon");
    }
    else if (player.tyre < 60.0)
    {
        advice.push_back("Tires at " + to_string((int)player.tyre) + "% - managing well");
    }

    // Position advice

    if (playerPos > 1)
    {
        // Find gap to car ahead
        for (int i = 0; i < field.size(); ++i)
        {
            if (field[i].currentPos == playerPos - 1)
            {
                double gap = field[i].cumulativeTime - player.cumulativeTime;
                if (gap < 3.0)
                {
                    advice.push_back("Gap to P" + to_string(playerPos - 1) + ": " + to_string(gap).substr(0, 3) + "s - within DRS!");
                }
                break;
            }
        }
    }

    if (playerPos < field.size())
    {
        // Find gap to car behind
        for (int i = 0; i < field.size(); ++i)
        {
            if (field[i].currentPos == playerPos + 1)
            {
                double gap = player.cumulativeTime - field[i].cumulativeTime;
                if (gap < 2.0)
                {
                    advice.push_back("Car behind closing - " + to_string(gap).substr(0, 3) + "s gap!");
                }
                break;
            }
        }
    }

    // Lap-based advice

    if (lap == totalLaps - 2)
    {
        advice.push_back("2 laps to go - push for final positions!");
    }

    if (lap > totalLaps - 5 && player.tyre > 60.0)
    {
        advice.push_back("Fresh tires advantage - attack now!");
    }

    if (advice.empty())
    {
        return "Maintaining good pace. Keep consistent laps.";
    }

    // Return random advice from the list
    return advice[uniform_int_distribution<int>(0, advice.size() - 1)(rng)];
}

// run race funtion

void runRace(const Driver &playerDriver, const string &playerName, const Track &track)
{
    clearScreen();
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│  " << track.name;
    for (int i = 0; i < 35 - track.name.length(); ++i)
        cout << " ";
    cout << "│\n";
    cout << "├─────────────────────────────────────┤\n";
    cout << "│                                     │\n";
    cout << "│        GRID FORMATION               │\n";
    cout << "│        Starting positions...        │\n";
    cout << "│                                     │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Grid is forming...\n";
    pressAnyKey();

    auto field = makeField(playerDriver, playerName);
    int fieldSize = (int)field.size();
    int playerIndex = 0;
    int totalLaps = 25;

    // Starting positions

    for (int i = 0; i < fieldSize; ++i)
    {
        field[i].cumulativeTime = i * 3.0;
        field[i].startingPos = i + 1;
    }
    recomputePositions(field);

    int playerMode = -1;
    double fastestLapTime = 1e9;
    string fastestLapHolder = "";
    int lastPlayerPos = field[playerIndex].currentPos;

    for (int lap = 1; lap <= totalLaps; ++lap)
    {
        clearScreen();
        cout << "┌──────────────────────────────────────────┐\n";
        printf("│    %-8s | LAP %2d/25 | POS: P%-2d       │\n",
               track.name.substr(0, 8).c_str(), lap, field[playerIndex].currentPos);
        cout << "├──────────────────────────────────────────┤\n";
        cout << "│                                          │\n";

        // Car Status

        Racer &p = field[playerIndex];
        cout << "│    📊 YOUR CAR                           │\n";
        printf("│    🛞 Tyres: %3d%%   🔧 Car: %3d%%          │\n", (int)p.tyre, (int)p.vehicle);
        printf("│    ⏱️ Last Lap: %-12s              │\n", formatTime(p.lastLapTime).c_str());
        if (fastestLapTime < 1e9)
        {
            printf("│    🏆 Fastest: %-12s (%-.3s)        │\n", formatTime(fastestLapTime).c_str(), fastestLapHolder.c_str());
        }
        else
        {
            cout << "│    🏆 Fastest: --:--.---                 │\n";
        }
        cout << "│                                          │\n";

        // LIVE GAP TIMES

        cout << "│    📡 LIVE GAPS                          │\n";

        // Find player position

        int playerPos = field[playerIndex].currentPos;
        double playerTime = field[playerIndex].cumulativeTime;

        // Show car ahead

        if (playerPos > 1)
        {
            for (int i = 0; i < fieldSize; ++i)
            {
                if (field[i].currentPos == playerPos - 1)
                {
                    double gap = field[i].cumulativeTime - playerTime;
                    string status = (gap < 2.0) ? "← CATCHING " : "← STABLE";
                    printf("│     P%d %-8s +%.1fs %-10s       │\n", playerPos - 1, field[i].displayName.substr(0, 8).c_str(), gap, status.c_str());
                    break;
                }
            }
        }
        else
        {
            cout << "│     LEADING THE RACE! 🏁                 │\n";
        }

        // Show car behind

        if (playerPos < fieldSize)
        {
            for (int i = 0; i < fieldSize; ++i)
            {
                if (field[i].currentPos == playerPos + 1)
                {
                    double gap = playerTime - field[i].cumulativeTime;
                    string status = (gap < 1.5) ? "← DEFEND! " : "← SAFE";
                    printf("│     P%d %-8s -%.1fs %-10s         │\n", playerPos + 1, field[i].displayName.substr(0, 8).c_str(), gap, status.c_str());
                    break;
                }
            }
        }
        else
        {
            cout << "│     NO PRESSURE FROM BEHIND           │\n";
        }

        cout << "│                                          │\n";

        // RACE ENGINEER ADVICE

        string engineerAdvice = getEngineerAdvice(p, lap, totalLaps, playerPos, field);
        cout << "│    🎙️ ENGINEER ADVICE                     │\n";

        // Use multi-line display for engineer advice

        int maxLineLength = 33;
        if (engineerAdvice.length() <= maxLineLength)
        {
            cout << "│    \"" << engineerAdvice;
            for (int i = 0; i < maxLineLength - engineerAdvice.length(); ++i)
                cout << " ";
            cout << "\"   │\n";
        }
        else
        {
            // Multi-line display
            vector<string> lines;
            string currentLine = "";
            stringstream ss(engineerAdvice);
            string word;

            while (ss >> word)
            {
                if (currentLine.length() + word.length() + 1 <= maxLineLength)
                {
                    if (!currentLine.empty())
                        currentLine += " ";
                    currentLine += word;
                }
                else
                {
                    lines.push_back(currentLine);
                    currentLine = word;
                }
            }
            if (!currentLine.empty())
                lines.push_back(currentLine);

            for (int i = 0; i < lines.size(); ++i)
            {
                if (i == 0)
                {
                    cout << "│    \"" << lines[i];
                    for (int j = 0; j < maxLineLength - lines[i].length(); ++j)
                        cout << " ";
                    cout << "\"    │\n";
                }
                else
                {
                    cout << "│      " << lines[i];
                    for (int j = 0; j < maxLineLength - lines[i].length(); ++j)
                        cout << " ";
                    cout << "     │\n";
                }
            }
        }

        cout << "│                                          │\n";

        // Commentary

        string comment = generateCommentary(p, lastPlayerPos, p.currentPos, p.tyre, p.inPitThisLap, lap, totalLaps, p.lastLapTime);

        cout << "│    🎙️ ENGINEER                            │\n";

        // Split long comments into multiple lines (using same maxLineLength)

        if (comment.length() <= maxLineLength)
        {
            // Single line comment

            cout << "│    \"" << comment;
            for (int i = 0; i < maxLineLength - comment.length(); ++i)
                cout << " ";
            cout << "\"      │\n";
        }
        else
        {
            // Multi-line comment
            vector<string> lines;
            string currentLine = "";
            stringstream ss(comment);
            string word;

            while (ss >> word)
            {
                if (currentLine.length() + word.length() + 1 <= maxLineLength)
                {
                    if (!currentLine.empty())
                        currentLine += " ";
                    currentLine += word;
                }
                else
                {
                    lines.push_back(currentLine);
                    currentLine = word;
                }
            }
            if (!currentLine.empty())
                lines.push_back(currentLine);

            // Display each line

            for (int i = 0; i < lines.size(); ++i)
            {
                if (i == 0)
                {
                    cout << "│    \"" << lines[i];
                    for (int j = 0; j < maxLineLength - lines[i].length(); ++j)
                        cout << " ";
                    cout << "\"  │\n";
                }
                else
                {
                    cout << "│      " << lines[i];
                    for (int j = 0; j < maxLineLength - lines[i].length(); ++j)
                        cout << " ";
                    cout << "   │\n";
                }
            }
        }

        // Add extra space if multi-line comment

        if (comment.length() > maxLineLength)
        {
            cout << "│                                          │\n";
        }

        cout << "│                                          │\n";

        // Strategy decision

        bool isDecisionLap = (lap % 3 == 1); // Every 3 laps: 1, 4, 7, 10, 13, 16, 19, 22
        int playerAction = -1;

        if (isDecisionLap)
        {
            cout << "│    💡 STRATEGY                           │\n";
            cout << "│    1. PUSH  🔥  (-0.5s, -8% tyres)       │\n";
            cout << "│    2. SAVE  🧊  (+0.3s, -3% tyres)       │\n";
            cout << "│    3. PIT   ⛽  (+" << track.pitStopTime << "s, fresh tyres)      │\n";
            cout << "│                                          │\n";
            cout << "└──────────────────────────────────────────┘\n";
            cout << "Enter choice (1-3): ";

            string input;
            getline(cin, input);
            playerAction = clampVal(stoi(input), 1, 3);
            if (playerAction == 1)
                playerMode = 1;
            else if (playerAction == 2)
                playerMode = 0;
        }
        else
        {
            cout << "│  💡 STRATEGY: ";
            if (playerMode == 1)
                cout << "PUSHING 🔥                 │\n";
            else if (playerMode == 0)
                cout << "SAVING 🧊                │\n";
            else
                cout << "BALANCED ⚖️                 │\n";
            cout << "│                                          │\n";
            cout << "└──────────────────────────────────────────┘\n";
            if (lap < totalLaps)
                pressAnyKey();
        }

        // Simulate lap

        for (int i = 0; i < fieldSize; ++i)
        {
            bool willPit = false;
            if (i == playerIndex && isDecisionLap && playerAction == 3)
                willPit = true;

            int mode = (i == playerIndex) ? playerMode : aiChooseStrategy(field[i]);
            if (i != playerIndex && field[i].tyre < 30.0)
                willPit = true;

            double lapTime = computeLapTimeSeconds(field[i], track, mode, i == playerIndex);

            if (willPit)
            {
                lapTime += track.pitStopTime;
                field[i].pitStops++;
                field[i].inPitThisLap = true;
            }
            else
            {
                field[i].inPitThisLap = false;
            }

            field[i].lastLapTime = lapTime;
            field[i].cumulativeTime += lapTime;

            if (lapTime < field[i].fastestLap)
                field[i].fastestLap = lapTime;
            if (lapTime < fastestLapTime)
            {
                fastestLapTime = lapTime;
                fastestLapHolder = field[i].displayName;
            }

            applyWearAndDamage(field[i], mode, willPit);
        }

        lastPlayerPos = field[playerIndex].currentPos;
        recomputePositions(field);

        if (lap == totalLaps)
        {
            cout << "\nFinal lap complete! Race finished!\n";
            pressAnyKey();
        }
    }

    // Results

    clearScreen();
    cout << "┌──────────────────────────────────────────┐\n";
    cout << "│          🏁 RACE CLASSIFICATION          │\n";
    cout << "│          " << track.name;
    for (int i = 0; i < 21 - track.name.length(); ++i)
        cout << " ";
    cout << "         │\n";
    cout << "├──────────────────────────────────────────┤\n";
    cout << "│                                          │\n";

    // Create and sort indices properly
    vector<int> idx(field.size());
    for (int i = 0; i < field.size(); ++i)
    {
        idx[i] = i;
    }

    // Sort by cumulativeTime
    sort(idx.begin(), idx.end(), [&](int a, int b)
         { return field[a].cumulativeTime < field[b].cumulativeTime; });

    for (int p = 0; p < (int)idx.size(); ++p)
    {
        int i = idx[p];
        string medal = "  ";
        if (p == 0)
            medal = "🥇";
        else if (p == 1)
            medal = "🥈";
        else if (p == 2)
            medal = "🥉";

        if (i == playerIndex)
        {
            printf("│     %s P%d. YOU%-15s%-9s   │\n", medal.c_str(), p + 1, "", formatTime(field[i].cumulativeTime).c_str());
        }
        else
        {
            printf("│     %s P%d. %-18s%-9s   │\n", medal.c_str(), p + 1, field[i].displayName.substr(0, 15).c_str(),
                   formatTime(field[i].cumulativeTime).c_str());
        }
    }

    cout << "│                                          │\n";
    printf("│    🏅 Fastest Lap: %-12s (%-8s)   │\n", formatTime(fastestLapTime).c_str(), fastestLapHolder.substr(0, 8).c_str());
    printf("│    🛞 Your Pit Stops: %-2d                 │\n", field[playerIndex].pitStops);
    printf("│    📈 Position: P%d → P%-2d                 │\n", field[playerIndex].startingPos, field[playerIndex].currentPos);
    cout << "│                                          │\n";
    cout << "└──────────────────────────────────────────┘\n";
    pressAnyKey();
}

// ---------- Main Function ----------

int main()
{
    SetConsoleOutputCP(65001); // UTF-8 code page
    while (true)
    {
        clearScreen();
        cout << "┌─────────────────────────────────────┐\n";
        cout << "│           🏎️ F1 2025 TERMINAL        │\n";
        cout << "│                RACER                │\n";
        cout << "├─────────────────────────────────────┤\n";
        cout << "│                                     │\n";
        cout << "│   1. 🏁 QUICK RACE                  │\n";
        cout << "│   2. 📖 ABOUT                       │\n";
        cout << "│   3. ❌ EXIT                        │\n";
        cout << "│                                     │\n";
        cout << "└─────────────────────────────────────┘\n";
        cout << "Enter choice (1-3): ";

        string input;
        getline(cin, input);

        if (input == "1")
        {
            string team = getTeamSelection();
            if (team == "")
                continue;

            Driver driver = getDriverSelection(team);
            if (driver.name == "")
                continue;

            string track = getTrackSelection();
            if (track == "")
                continue;

            runRace(driver, driver.name, tracks[track]);
        }
        else if (input == "2")
        {
            showAbout();
        }
        else if (input == "3")
        {
            break;
        }
    }

    cout << "Thanks for playing F1 Terminal Racer! 🏁\n";
    return 0;

}
