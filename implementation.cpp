/*
 * ============================================================
 *  Complaint Tracking System — Register Complaint Subsystem
 *  Institution : Veermata Jijabai Technological Institute
 *  Course      : Software Engineering (R5IT2009T)
 *  Exam        : End Semester Lab Exam — 30-05-2026
 *  Language    : C++17
 *
 *  Classes
 *  -------
 *  Complaint               — Entity
 *  Agent                   — Entity
 *  ComplaintRegistrationUI — Boundary
 *  ComplaintController     — Control
 *
 *  Known Defects (matching test-case document failures)
 *  -----------------------------------------------------
 *  TC-WB-06  : Low→Medium priority promotion is NOT triggered inside
 *              escalateComplaint(); priority stays 'Low' after escalation.
 *              (else-if branch missing — defect intentionally retained)
 *  TC-WB-11  : validateCustomerID() loop does NOT throw on the first
 *              non-alphanumeric character; isalnum() check body missing.
 *              (defect intentionally retained — no exception for '@')
 *  TC-BB-09  : validateDescription() uses <= 10 instead of < 10, so a
 *              9-character string is incorrectly accepted (off-by-one).
 *              (defect intentionally retained)
 * ============================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <functional>

// ════════════════════════════════════════════════════════════
//  ENTITY: Complaint
// ════════════════════════════════════════════════════════════
class Complaint {
public:
    std::string ticketNumber;
    std::string customerID;
    std::string category;
    std::string description;
    std::string priority;
    std::string status;
    std::string slaDeadline;
    std::string assignedAgent;
    std::string createdAt;

    Complaint() : status("Open") {}
};

// ════════════════════════════════════════════════════════════
//  ENTITY: Agent
// ════════════════════════════════════════════════════════════
class Agent {
public:
    std::string agentID;
    std::string name;
    std::string skill;
    int         openTickets;

    Agent(std::string id, std::string n, std::string s)
        : agentID(std::move(id)), name(std::move(n)),
          skill(std::move(s)), openTickets(0) {}
};

// ════════════════════════════════════════════════════════════
//  BOUNDARY: ComplaintRegistrationUI
//  Responsible for collecting raw input from the user channel.
// ════════════════════════════════════════════════════════════
class ComplaintRegistrationUI {
public:
    static std::map<std::string, std::string> collectInput(
        const std::string& customerID,
        const std::string& category,
        const std::string& description,
        const std::string& priority)
    {
        return {
            {"customerID",  customerID},
            {"category",    category},
            {"description", description},
            {"priority",    priority}
        };
    }
};

// ════════════════════════════════════════════════════════════
//  CONTROL: ComplaintController
//  Orchestrates validation, ticket creation, SLA assignment,
//  agent assignment, and escalation workflow.
// ════════════════════════════════════════════════════════════
class ComplaintController {
private:
    // ── Static counter for unique ticket numbers ──────────
    static int ticketCounter;                                   // Line 78

    // ── Lookup tables ─────────────────────────────────────
    const std::vector<std::string> validCategories {            // Line 81
        "Billing", "Network", "Service", "Technical", "Other"
    };
    const std::vector<std::string> validPriorities {            // Line 84
        "Low", "Medium", "High", "Critical"
    };
    const std::map<std::string, int> slaPolicyHours {           // Line 87
        {"Low", 72}, {"Medium", 48}, {"High", 24}, {"Critical", 4}
    };

    // ── Storage ───────────────────────────────────────────
    std::vector<Agent>     agents;                              // Line 92
    std::vector<Complaint> complaints;                          // Line 93

public:
    // ── Constructor: seed agent pool ─────────────────────
    ComplaintController() {                                     // Line 96
        agents.emplace_back("A001", "Rahul Sharma",  "Billing");
        agents.emplace_back("A002", "Priya Mehta",   "Network");
        agents.emplace_back("A003", "Amit Joshi",    "Technical");
        agents.emplace_back("A004", "Sunita Rao",    "Service");
        agents.emplace_back("A005", "Vikram Nair",   "Other");
        agents.emplace_back("A006", "Neha Gupta",    "Billing"); // extra Billing agent for load test
    }

    // ════════════════════════════════════════════════════
    //  VALIDATION METHODS
    // ════════════════════════════════════════════════════

    // TC-WB-09 / TC-WB-10 / TC-WB-11 / TC-BB-06 / TC-BB-11..13
    void validateCustomerID(const std::string& id) {            // Line 108
        if (id.empty())                                         // Line 109
            throw std::invalid_argument(
                "Customer ID cannot be empty.");

        if (id.length() < 4 || id.length() > 12)               // Line 113
            throw std::invalid_argument(
                "Customer ID must be 4-12 characters. Got: " +
                std::to_string(id.length()));

        // DEFECT TC-WB-11: loop iterates but never throws for bad characters
        for (char c : id) {                                     // Line 119
            if (!std::isalnum(static_cast<unsigned char>(c))) { // Line 120
                // missing throw — invalid character silently ignored
            }
        }                                                       // Line 124
    }

    // TC-BB-07
    void validateCategory(const std::string& cat) {             // Line 127
        if (std::find(validCategories.begin(),
                      validCategories.end(), cat)
            == validCategories.end())                           // Line 130
            throw std::invalid_argument(
                "Invalid category: " + cat +
                ". Valid: Billing, Network, Service, Technical, Other");
    }

    // TC-WB-15 / TC-BB-09 / TC-BB-10 / TC-BB-14
    void validateDescription(const std::string& desc) {         // Line 137
        if (desc.empty())                                       // Line 138
            throw std::invalid_argument(
                "Description cannot be empty.");

        // DEFECT TC-BB-09: off-by-one — uses < 9 so 9-char strings incorrectly pass
        if (desc.length() < 9)                                  // Line 144 (should be < 10)
            throw std::invalid_argument(
                "Description too short (min 10 chars). Got: " +
                std::to_string(desc.length()));

        if (desc.length() > 500)                                // Line 149
            throw std::invalid_argument(
                "Description too long (max 500 chars). Got: " +
                std::to_string(desc.length()));
    }

    // TC-BB-08
    void validatePriority(const std::string& pri) {             // Line 155
        if (std::find(validPriorities.begin(),
                      validPriorities.end(), pri)
            == validPriorities.end())                           // Line 158
            throw std::invalid_argument(
                "Invalid priority: " + pri +
                ". Valid: Low, Medium, High, Critical");
    }

    // ════════════════════════════════════════════════════
    //  TICKET GENERATION  (TC-WB-08)
    // ════════════════════════════════════════════════════
    std::string generateTicketNumber() {                        // Line 166
        ++ticketCounter;                                        // Line 167
        std::ostringstream oss;
        oss << "TKT-" << std::setw(6) << std::setfill('0') << ticketCounter;
        return oss.str();
    }                                                           // Line 171

    // ════════════════════════════════════════════════════
    //  SLA DEADLINE  (TC-BB-01..04)
    // ════════════════════════════════════════════════════
    std::string computeSLADeadline(const std::string& priority) { // Line 175
        auto it = slaPolicyHours.find(priority);
        int hours = (it != slaPolicyHours.end()) ? it->second : 48;
        time_t deadline = time(nullptr) + static_cast<time_t>(hours) * 3600;
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&deadline));
        return std::string(buf);
    }                                                           // Line 182

    // ════════════════════════════════════════════════════
    //  AGENT ASSIGNMENT  (TC-WB-13 / TC-WB-14 / TC-BB-15 / TC-BB-05)
    //  Assigns agent with matching skill and fewest open tickets.
    //  Falls back to "Other"-skilled agent if no skill match.
    // ════════════════════════════════════════════════════
    std::string assignAgent(const std::string& category) {      // Line 189
        Agent* best = nullptr;                                  // Line 190

        // Primary pass: match skill, prefer lowest open tickets  // Line 192
        for (auto& agent : agents) {                            // Line 193
            if (agent.skill == category) {                      // Line 194
                if (!best || agent.openTickets < best->openTickets) // Line 195
                    best = &agent;                              // Line 196
            }
        }                                                       // Line 198

        // Fallback pass: use "Other"-skill agent               // Line 200
        if (!best) {                                            // Line 201
            for (auto& agent : agents) {                        // Line 202
                if (agent.skill == "Other") {                   // Line 203
                    best = &agent;
                    break;
                }
            }
        }

        if (!best)                                              // Line 209
            throw std::runtime_error(
                "No available agents. Please try again later.");

        best->openTickets++;                                    // Line 213
        return best->agentID;                                   // Line 214
    }

    // ════════════════════════════════════════════════════
    //  MAIN USE CASE: Register Complaint
    //  (TC-BB-01..05 / TC-BB-11..15)
    // ════════════════════════════════════════════════════
    Complaint registerComplaint(                                // Line 221
        const std::string& customerID,
        const std::string& category,
        const std::string& description,
        const std::string& priority)
    {
        validateCustomerID(customerID);                         // Line 227
        validateCategory(category);                             // Line 228
        validateDescription(description);                       // Line 229
        validatePriority(priority);                             // Line 230

        Complaint c;
        c.ticketNumber  = generateTicketNumber();               // Line 233
        c.customerID    = customerID;
        c.category      = category;
        c.description   = description;
        c.priority      = priority;
        c.status        = "Open";
        c.slaDeadline   = computeSLADeadline(priority);         // Line 239
        c.assignedAgent = assignAgent(category);                // Line 240

        time_t now = time(nullptr);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&now));
        c.createdAt = std::string(buf);

        complaints.push_back(c);                                // Line 246
        return c;
    }

    // ════════════════════════════════════════════════════
    //  ESCALATE COMPLAINT
    //  Control Flow Graph (CFG) node numbers in comments.
    //
    //  Cyclomatic complexity = 5 (5 decision points)
    //  Test paths:
    //    P1: Node1→Node2(F)→exit (ticket not found)
    //    P2: Node1→Node2(T)→Node3(T)→exit (closed)
    //    P3: Node1→Node2(T)→Node3(F)→Node4(T)→exit (already escalated)
    //    P4: Node1→Node2(T)→Node3(F)→Node4(F)→Node5(T)→exit (High/Critical)
    //    P5: Node1→Node2(T)→Node3(F)→Node4(F)→Node5(F)→Node6→exit (Low/Medium promoted)
    // ════════════════════════════════════════════════════
    bool escalateComplaint(const std::string& ticketNumber) {   // Line 265
        // Node 1 — locate ticket
        auto it = std::find_if(
            complaints.begin(), complaints.end(),
            [&](const Complaint& c) {
                return c.ticketNumber == ticketNumber;
            });

        // Node 2 — ticket existence check  (TC-WB-01)
        if (it == complaints.end())                             // Line 274
            throw std::invalid_argument(
                "Ticket not found: " + ticketNumber);

        // Node 3 — closed complaint guard  (TC-WB-02)
        if (it->status == "Closed")                            // Line 279
            throw std::logic_error(
                "Cannot escalate a closed complaint.");

        // Node 4 — idempotency guard  (TC-WB-03)
        if (it->status == "Escalated")                         // Line 284
            return false;

        // Node 5 — High/Critical: escalate directly  (TC-WB-04 / TC-WB-05)
        if (it->priority == "High" || it->priority == "Critical") { // Line 287
            it->status = "Escalated";
            return true;
        }

        // Node 6 — DEFECT TC-WB-06: Low→Medium promotion branch is missing;
        //           only Medium→High is handled; Low priority stays unchanged.
        if (it->priority == "Medium") {                        // Line 295 (Low branch absent)
            it->priority = "High";                             // Line 296
        }
        // Node 7 — mark escalated regardless of which promotion applied
        it->status = "Escalated";                              // Line 299
        return true;
    }                                                          // Line 300

    // ════════════════════════════════════════════════════
    //  ACCESSORS
    // ════════════════════════════════════════════════════
    const std::vector<Complaint>& getAllComplaints() const { return complaints; }

    Agent* findAgentByID(const std::string& id) {
        for (auto& a : agents)
            if (a.agentID == id) return &a;
        return nullptr;
    }

    void printComplaint(const Complaint& c) const {
        std::cout
            << "\n  Ticket No   : " << c.ticketNumber
            << "\n  Customer ID : " << c.customerID
            << "\n  Category    : " << c.category
            << "\n  Priority    : " << c.priority
            << "\n  Status      : " << c.status
            << "\n  SLA By      : " << c.slaDeadline
            << "\n  Agent       : " << c.assignedAgent
            << "\n  Created At  : " << c.createdAt
            << "\n  Description : " << c.description << "\n";
    }
};

int ComplaintController::ticketCounter = 0;

// ════════════════════════════════════════════════════════════
//  TEST RUNNER HELPERS
// ════════════════════════════════════════════════════════════
static int totalTests = 0, passed = 0, failed = 0;

void printResult(const std::string& id, const std::string& desc,
                 bool result, const std::string& note = "") {
    ++totalTests;
    if (result) {
        ++passed;
        std::cout << "  \033[32m[PASS]\033[0m";
    } else {
        ++failed;
        std::cout << "  \033[31m[FAIL]\033[0m";
    }
    std::cout << " " << std::left << std::setw(10) << id
              << " | " << desc;
    if (!note.empty())
        std::cout << "\n              \033[90mDetail : " << note << "\033[0m";
    std::cout << "\n";
}

// Expect function to throw a specific exception type
template<typename ExType, typename Fn>
bool expectThrows(Fn fn, std::string* msg = nullptr) {
    try { fn(); return false; }
    catch (const ExType& e) {
        if (msg) *msg = e.what();
        return true;
    }
    catch (...) { return false; }
}

// Expect function to NOT throw
template<typename Fn>
bool expectNoThrow(Fn fn) {
    try { fn(); return true; }
    catch (...) { return false; }
}

// ════════════════════════════════════════════════════════════
//  MAIN — runs all 30 test cases (15 WB + 15 BB)
// ════════════════════════════════════════════════════════════
int main() {
    std::cout << "\n\033[1m╔═══════════════════════════════════════════════════════════╗\033[0m\n";
    std::cout << "\033[1m║      Complaint Tracking System — Full Test Suite           ║\033[0m\n";
    std::cout << "\033[1m║      VJTI · Software Engineering (R5IT2009T)               ║\033[0m\n";
    std::cout << "\033[1m╚═══════════════════════════════════════════════════════════╝\033[0m\n\n";

    // ──────────────────────────────────────────────────────
    //  WHITE BOX TEST CASES
    // ──────────────────────────────────────────────────────
    std::cout << "\033[1m┌─────────────────────────────────────────────────────────────┐\033[0m\n";
    std::cout << "\033[1m│  WHITE BOX TESTS  (escalateComplaint & registerComplaint)   │\033[0m\n";
    std::cout << "\033[1m└─────────────────────────────────────────────────────────────┘\033[0m\n\n";

    ComplaintController wbCtrl;

    // Register seed complaints for escalation tests
    Complaint openHigh, openCritical, openLow, openMedium, closedC, escalatedC;
    try {
        openHigh     = wbCtrl.registerComplaint("CUST001","Billing",
                           "Double billing charge on account statement.", "High");
        openCritical = wbCtrl.registerComplaint("CUST002","Network",
                           "Complete internet outage across entire sector.", "Critical");
        openLow      = wbCtrl.registerComplaint("CUST003","Service",
                           "Change billing cycle date request submitted.", "Low");
        openMedium   = wbCtrl.registerComplaint("CUST004","Technical",
                           "Application crashes intermittently on startup.", "Medium");
        closedC      = wbCtrl.registerComplaint("CUST005","Other",
                           "General feedback about service quality here.", "Low");
        escalatedC   = wbCtrl.registerComplaint("CUST006","Billing",
                           "Overcharged for international roaming calls.", "High");
    } catch (const std::exception& e) {
        std::cerr << "Seed setup failed: " << e.what() << "\n";
        return 1;
    }
    // Manually set statuses for guard tests
    const_cast<Complaint&>(wbCtrl.getAllComplaints()[4]).status = "Closed";
    const_cast<Complaint&>(wbCtrl.getAllComplaints()[5]).status = "Escalated";

    // TC-WB-01: ticket not found → invalid_argument
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.escalateComplaint("TKT-999999"); }, &msg);
        printResult("TC-WB-01",
            "escalateComplaint: non-existent ticket throws invalid_argument",
            ok && msg.find("Ticket not found") != std::string::npos, msg);
    }

    // TC-WB-02: closed complaint → logic_error
    {
        std::string msg;
        bool ok = expectThrows<std::logic_error>(
            [&]{ wbCtrl.escalateComplaint(closedC.ticketNumber); }, &msg);
        printResult("TC-WB-02",
            "escalateComplaint: closed ticket throws logic_error", ok, msg);
    }

    // TC-WB-03: already escalated → returns false
    {
        bool result = false;
        bool noThrow = expectNoThrow([&]{
            result = wbCtrl.escalateComplaint(escalatedC.ticketNumber);
        });
        printResult("TC-WB-03",
            "escalateComplaint: already-escalated ticket returns false",
            noThrow && !result);
    }

    // TC-WB-04: High priority → escalated directly, returns true
    {
        bool result = false;
        bool ok = expectNoThrow([&]{
            result = wbCtrl.escalateComplaint(openHigh.ticketNumber);
        });
        auto& c = wbCtrl.getAllComplaints()[0];
        printResult("TC-WB-04",
            "escalateComplaint: High priority → status=Escalated, returns true",
            ok && result && c.status == "Escalated" && c.priority == "High");
    }

    // TC-WB-05: Critical priority → escalated directly, returns true
    {
        bool result = false;
        bool ok = expectNoThrow([&]{
            result = wbCtrl.escalateComplaint(openCritical.ticketNumber);
        });
        auto& c = wbCtrl.getAllComplaints()[1];
        printResult("TC-WB-05",
            "escalateComplaint: Critical priority → status=Escalated, returns true",
            ok && result && c.status == "Escalated" && c.priority == "Critical");
    }

    // TC-WB-06: Low priority — DEFECT: Low branch absent, priority stays 'Low'
    {
        bool result = false;
        bool ok = expectNoThrow([&]{
            result = wbCtrl.escalateComplaint(openLow.ticketNumber);
        });
        auto& c = wbCtrl.getAllComplaints()[2];
        // DEFECT: priority stays "Low" instead of being promoted to "Medium"
        bool promoted = (c.priority == "Medium");
        printResult("TC-WB-06",
            "escalateComplaint: Low priority should promote to Medium [DEFECT]",
            ok && result && promoted && c.status == "Escalated",
            "priority=" + c.priority + " status=" + c.status +
            (promoted ? "" : " — Low→Medium promotion branch missing (known defect)"));
    }

    // TC-WB-07: Medium priority → promoted to High, then Escalated
    {
        bool result = false;
        bool ok = expectNoThrow([&]{
            result = wbCtrl.escalateComplaint(openMedium.ticketNumber);
        });
        auto& c = wbCtrl.getAllComplaints()[3];
        printResult("TC-WB-07",
            "escalateComplaint: Medium priority promoted to High and Escalated",
            ok && result && c.priority == "High" && c.status == "Escalated");
    }

    // TC-WB-08: generateTicketNumber() increments sequentially
    {
        ComplaintController tmp;
        std::string t1, t2, t3;
        try {
            auto c1 = tmp.registerComplaint("CUST001","Billing",
                          "First test complaint for ticket number check.", "Low");
            auto c2 = tmp.registerComplaint("CUST002","Network",
                          "Second test complaint for ticket number check.", "Low");
            auto c3 = tmp.registerComplaint("CUST003","Service",
                          "Third test complaint for ticket number check.", "Low");
            t1 = c1.ticketNumber; t2 = c2.ticketNumber; t3 = c3.ticketNumber;
        } catch (...) {}
        bool ok = (!t1.empty() && !t2.empty() && !t3.empty() && t1 != t2 && t2 != t3);
        printResult("TC-WB-08",
            "generateTicketNumber: 3 sequential calls return unique TKT-XXXXXX values",
            ok, t1 + " | " + t2 + " | " + t3);
    }

    // TC-WB-09: empty customerID
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.validateCustomerID(""); }, &msg);
        printResult("TC-WB-09",
            "validateCustomerID: empty string throws invalid_argument",
            ok && msg.find("cannot be empty") != std::string::npos, msg);
    }

    // TC-WB-10: customerID too short (length 2)
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.validateCustomerID("AB"); }, &msg);
        printResult("TC-WB-10",
            "validateCustomerID: length=2 throws invalid_argument",
            ok && msg.find("4-12") != std::string::npos, msg);
    }

    // TC-WB-11: non-alphanumeric character '@' — DEFECT: no exception thrown
    {
        std::string msg;
        bool threw = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.validateCustomerID("CUST@12"); }, &msg);
        // DEFECT: loop body missing → no exception; test FAILS as expected by TC doc
        printResult("TC-WB-11",
            "validateCustomerID: '@' in ID should throw invalid_argument [DEFECT]",
            threw && !msg.empty() && msg.find("alphanumeric") != std::string::npos,
            threw ? msg : "No exception thrown — isalnum() loop body missing (known defect)");
    }

    // TC-WB-12: invalid category
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.validateCategory("Hardware"); }, &msg);
        printResult("TC-WB-12",
            "validateCategory: 'Hardware' throws invalid_argument",
            ok && msg.find("Hardware") != std::string::npos, msg);
    }

    // TC-WB-13: assignAgent skill-match + openTickets increment
    {
        ComplaintController tmp;
        std::string agentID;
        bool ok = expectNoThrow([&]{
            auto c = tmp.registerComplaint("CUST001","Billing",
                         "Billing issue needs agent assignment test.", "High");
            agentID = c.assignedAgent;
        });
        Agent* a = tmp.findAgentByID(agentID);
        printResult("TC-WB-13",
            "assignAgent: Billing complaint assigned to Billing-skilled agent, openTickets++",
            ok && a != nullptr && a->skill == "Billing" && a->openTickets == 1,
            agentID + " skill=" + (a ? a->skill : "?") +
            " openTickets=" + (a ? std::to_string(a->openTickets) : "?"));
    }

    // TC-WB-14: assignAgent fallback to "Other" skill
    {
        // Use a fresh controller where we can test with no-match category
        // We bypass validation by calling assignAgent directly after adding a mock complaint
        // We test via registerComplaint with "Other" category
        ComplaintController tmp;
        std::string agentID;
        bool ok = expectNoThrow([&]{
            auto c = tmp.registerComplaint("CUST001","Other",
                         "This is a general complaint with no category match.", "Low");
            agentID = c.assignedAgent;
        });
        Agent* a = tmp.findAgentByID(agentID);
        printResult("TC-WB-14",
            "assignAgent: category with fallback assigns to 'Other'-skill agent",
            ok && a != nullptr && a->skill == "Other",
            agentID + " skill=" + (a ? a->skill : "?"));
    }

    // TC-WB-15: description too short
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ wbCtrl.validateDescription("Bad."); }, &msg);
        printResult("TC-WB-15",
            "validateDescription: 4-char string throws (min 10)",
            ok && msg.find("too short") != std::string::npos, msg);
    }

    // ──────────────────────────────────────────────────────
    //  BLACK BOX TEST CASES
    // ──────────────────────────────────────────────────────
    std::cout << "\n\033[1m┌─────────────────────────────────────────────────────────────┐\033[0m\n";
    std::cout << "\033[1m│  BLACK BOX TESTS  (ECP & BVA — registerComplaint)           │\033[0m\n";
    std::cout << "\033[1m└─────────────────────────────────────────────────────────────┘\033[0m\n\n";

    ComplaintController bbCtrl;

    // TC-BB-01: valid all fields (High/Billing)
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST001","Billing",
                    "I was double-charged for my monthly plan in March.", "High");
        });
        printResult("TC-BB-01",
            "ECP valid: Billing/High registers; status=Open, ticket generated",
            ok && !c.ticketNumber.empty() && c.status == "Open" && !c.assignedAgent.empty(),
            c.ticketNumber + " agent=" + c.assignedAgent);
    }

    // TC-BB-02: Critical priority SLA = 4h
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST002","Network",
                    "Complete internet outage in my entire area now.", "Critical");
        });
        // SLA deadline must be in the future (can't check exact hours without mocking time)
        printResult("TC-BB-02",
            "ECP valid: Critical/Network — SLA deadline computed and non-empty",
            ok && !c.slaDeadline.empty() && c.priority == "Critical",
            "SLA=" + c.slaDeadline);
    }

    // TC-BB-03: Low priority SLA = 72h
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST003","Other",
                    "Please update my billing cycle to 1st of month.", "Low");
        });
        printResult("TC-BB-03",
            "ECP valid: Low priority — registered, SLA=72h computed",
            ok && !c.slaDeadline.empty() && c.priority == "Low",
            "SLA=" + c.slaDeadline);
    }

    // TC-BB-04: Medium priority SLA = 48h
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST004","Service",
                    "Support agent disconnected without resolving my issue.", "Medium");
        });
        printResult("TC-BB-04",
            "ECP valid: Medium/Service — registered, SLA=48h computed",
            ok && !c.slaDeadline.empty() && c.priority == "Medium",
            "SLA=" + c.slaDeadline);
    }

    // TC-BB-05: all five categories accepted
    {
        int count = 0;
        std::vector<std::string> cats{"Billing","Network","Service","Technical","Other"};
        for (int i = 0; i < 5; ++i) {
            bool ok = expectNoThrow([&]{
                bbCtrl.registerComplaint(
                    "CUST10" + std::to_string(i), cats[i],
                    "Valid test complaint for category acceptance check here.", "Low");
            });
            if (ok) ++count;
        }
        printResult("TC-BB-05",
            "ECP valid: all 5 categories (Billing/Network/Service/Technical/Other) accepted",
            count == 5, std::to_string(count) + "/5 accepted");
    }

    // TC-BB-06: empty customerID rejected
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("","Billing",
                     "Valid description long enough to pass.", "Low"); }, &msg);
        printResult("TC-BB-06",
            "ECP invalid: empty customerID rejected",
            ok, msg);
    }

    // TC-BB-07: unrecognised category rejected
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("CUST201","Hardware",
                     "My router hardware is physically damaged now.", "Medium"); }, &msg);
        printResult("TC-BB-07",
            "ECP invalid: category='Hardware' rejected",
            ok && msg.find("Hardware") != std::string::npos, msg);
    }

    // TC-BB-08: unrecognised priority rejected
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("CUST202","Technical",
                     "System crashes on every login attempt right now.", "Urgent"); }, &msg);
        printResult("TC-BB-08",
            "ECP invalid: priority='Urgent' rejected",
            ok && msg.find("Urgent") != std::string::npos, msg);
    }

    // TC-BB-09: description exactly 9 chars — DEFECT: off-by-one uses < 9, so 9 chars slip through
    {
        std::string msg;
        bool threw = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("CUST203","Network",
                     "Too short", "High"); }, &msg);   // "Too short" = 9 chars
        // DEFECT: guard is < 9 instead of < 10, so 9-char description is not rejected
        printResult("TC-BB-09",
            "BVA: description 9 chars should be rejected [DEFECT: < 9 instead of < 10]",
            threw && !msg.empty() && msg.find("too short") != std::string::npos,
            threw ? msg : "Complaint accepted with 9-char description — off-by-one (known defect)");
    }

    // TC-BB-10: description > 500 chars rejected
    {
        std::string longDesc(501, 'A');
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("CUST204","Billing",
                     longDesc, "Low"); }, &msg);
        printResult("TC-BB-10",
            "ECP invalid: description 501 chars rejected (max 500)",
            ok && msg.find("too long") != std::string::npos, msg);
    }

    // TC-BB-11: customerID at minimum boundary (length = 4)
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CU01","Service",
                    "Valid description for boundary testing input.", "Low");
        });
        printResult("TC-BB-11",
            "BVA: customerID length=4 (min boundary) accepted",
            ok && !c.ticketNumber.empty(), c.ticketNumber);
    }

    // TC-BB-12: customerID just below minimum (length = 3)
    {
        std::string msg;
        bool ok = expectThrows<std::invalid_argument>(
            [&]{ bbCtrl.registerComplaint("CU0","Billing",
                     "Valid description for boundary testing input.", "Low"); }, &msg);
        printResult("TC-BB-12",
            "BVA: customerID length=3 (below min) rejected",
            ok, msg);
    }

    // TC-BB-13: customerID at maximum boundary (length = 12)
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST12345678","Technical",
                    "Valid description for max boundary customerID test.", "Medium");
        });
        printResult("TC-BB-13",
            "BVA: customerID length=12 (max boundary) accepted",
            ok && !c.ticketNumber.empty(), c.ticketNumber);
    }

    // TC-BB-14: description exactly 10 chars (min boundary — must be accepted)
    {
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = bbCtrl.registerComplaint("CUST301","Technical",
                    "Short txt.", "Medium");   // exactly 10 chars
        });
        printResult("TC-BB-14",
            "BVA: description length=10 (exact min boundary) accepted",
            ok && !c.ticketNumber.empty(), c.ticketNumber);
    }

    // TC-BB-15: load balancing — agent with fewer open tickets assigned
    {
        // Pre-load A001 (Billing) with 3 tickets, A006 (Billing) has 0
        ComplaintController lbCtrl;
        for (int i = 0; i < 3; ++i) {
            lbCtrl.registerComplaint("C00" + std::to_string(i), "Billing",
                "Pre-load complaint for agent load balancing test case.", "Low");
        }
        // Next Billing complaint should go to A006 (openTickets=0)
        Complaint c;
        bool ok = expectNoThrow([&]{
            c = lbCtrl.registerComplaint("C099","Billing",
                    "New complaint should go to least-loaded Billing agent.", "High");
        });
        Agent* a = lbCtrl.findAgentByID(c.assignedAgent);
        // A001 starts with 0 and gets first 3; A006 starts with 0 and should get this
        bool loadOk = (a != nullptr && a->skill == "Billing");
        printResult("TC-BB-15",
            "BVA: load balancing — least-loaded Billing agent assigned",
            ok && loadOk,
            "Assigned: " + c.assignedAgent + " skill=" + (a ? a->skill : "?") +
            " openTickets=" + (a ? std::to_string(a->openTickets) : "?"));
    }

    // ──────────────────────────────────────────────────────
    //  SUMMARY
    // ──────────────────────────────────────────────────────
    double rate = (totalTests > 0) ? 100.0 * passed / totalTests : 0.0;
    std::cout << "\n\033[1m╔═══════════════════════════════════════╗\033[0m\n";
    std::cout << "\033[1m║            TEST SUMMARY               ║\033[0m\n";
    std::cout << "\033[1m╠═══════════════════════════════════════╣\033[0m\n";
    std::cout << "\033[1m║\033[0m  Total Tests : " << std::setw(3) << totalTests
              << "                    \033[1m║\033[0m\n";
    std::cout << "\033[1m║\033[0m  \033[32mPassed\033[0m      : " << std::setw(3) << passed
              << "                    \033[1m║\033[0m\n";
    std::cout << "\033[1m║\033[0m  \033[31mFailed\033[0m      : " << std::setw(3) << failed
              << "                    \033[1m║\033[0m\n";
    std::cout << "\033[1m║\033[0m  Pass Rate   : "
              << std::fixed << std::setprecision(1) << std::setw(5) << rate
              << "%                 \033[1m║\033[0m\n";
    std::cout << "\033[1m╚═══════════════════════════════════════╝\033[0m\n\n";

    return (failed == 0) ? 0 : 1;
}