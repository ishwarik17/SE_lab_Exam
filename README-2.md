# Complaint Tracking System — Register Complaint Subsystem

**Institution:** Veermata Jijabai Technological Institute (VJTI)  
**Course:** Software Engineering (R5IT2009T)  
**Semester:** IV, B.Tech Computer Engineering  
**Exam:** End Semester Lab Exam — 30-05-2026  
**Language:** C++17

---

## Project Overview

This project implements the **Register Complaint** subsystem of a Complaint Tracking System, designed following object-oriented principles. It supports complaint registration, SLA deadline computation, agent assignment, and complaint escalation.

The implementation is accompanied by a full test suite of **30 test cases** (15 White Box + 15 Black Box) and a **runtime code coverage tracker** that reports branch-level coverage after each run.

---

## File Structure

```
.
├── implementation.cpp               # Full source: classes + test suite + coverage tracker
├── Q2_TestCases_ComplaintRegister.xlsx  # Test case document (ECP, BVA, White Box)
└── Register_Complaint.pdf           # Sample output showing test results and coverage report
```

---

## Architecture

The system follows a classic **Boundary–Control–Entity** (BCE) pattern:

| Class | Type | Responsibility |
|---|---|---|
| `Complaint` | Entity | Holds complaint data (ticket no., customer ID, priority, SLA, status, agent) |
| `Agent` | Entity | Represents a support agent with skill and open-ticket count |
| `ComplaintRegistrationUI` | Boundary | Collects and packages raw user input |
| `ComplaintController` | Control | Orchestrates validation, ticket creation, SLA assignment, agent assignment, and escalation |

---

## Core Features

### 1. Complaint Registration (`registerComplaint`)
Accepts `customerID`, `category`, `description`, and `priority`. Runs all validations, generates a unique ticket number (`TKT-XXXXXX`), computes the SLA deadline, and assigns the best-fit agent.

### 2. Validation Rules

| Field | Rules |
|---|---|
| `customerID` | Non-empty; 4–12 characters; alphanumeric only |
| `category` | One of: `Billing`, `Network`, `Service`, `Technical`, `Other` |
| `description` | Non-empty; minimum 10 characters; maximum 500 characters |
| `priority` | One of: `Low`, `Medium`, `High`, `Critical` |

### 3. SLA Policy

| Priority | SLA Hours |
|---|---|
| Low | 72 h |
| Medium | 48 h |
| High | 24 h |
| Critical | 4 h |

### 4. Agent Assignment
Agents are matched by skill (category). When multiple agents share the same skill, the one with the **fewest open tickets** is selected (load balancing). Falls back to any `Other`-skilled agent if no skill match is found.

### 5. Escalation (`escalateComplaint`)
Follows a 7-node control flow graph (cyclomatic complexity = 5):

| Path | Condition | Outcome |
|---|---|---|
| P1 | Ticket not found | Throws `std::invalid_argument` |
| P2 | Ticket is `Closed` | Throws `std::logic_error` |
| P3 | Ticket already `Escalated` | Returns `false` |
| P4 | Priority is `High` or `Critical` | Sets status to `Escalated`, returns `true` |
| P5 | Priority is `Medium` | Promotes to `High`, sets `Escalated`, returns `true` |
| P5* | Priority is `Low` | ⚠️ **Known defect** — promotion to `Medium` missing (see below) |

---

## Known Defects

Three intentional defects are present in this build for testing purposes:

| ID | Location | Description |
|---|---|---|
| **TC-WB-06** | `escalateComplaint()` | `Low → Medium` priority promotion branch is absent; `Low` priority stays unchanged after escalation |
| **TC-WB-11** | `validateCustomerID()` | The `isalnum()` loop body has no `throw`; non-alphanumeric characters (e.g. `@`) are silently accepted |
| **TC-BB-09** | `validateDescription()` | Off-by-one error: guard uses `< 9` instead of `< 10`, so a 9-character description incorrectly passes |

---

## Test Suite

### White Box Tests (15 cases — `escalateComplaint` & `registerComplaint`)

Coverage types exercised: Statement (SC), Branch (BC), Path (PC), Condition (CC), Exception (EC).

| ID | Description | Coverage | Result |
|---|---|---|---|
| TC-WB-01 | Non-existent ticket throws `invalid_argument` | SC | PASS |
| TC-WB-02 | Closed ticket throws `logic_error` | BC | PASS |
| TC-WB-03 | Already-escalated ticket returns `false` | BC | PASS |
| TC-WB-04 | High priority → `Escalated`, returns `true` | BC | PASS |
| TC-WB-05 | Critical priority → `Escalated`, returns `true` | BC | PASS |
| TC-WB-06 | Low priority should promote to Medium | PC | **FAIL** (known defect) |
| TC-WB-07 | Medium priority promoted to High, then Escalated | PC | PASS |
| TC-WB-08 | Ticket counter increments sequentially | SC | PASS |
| TC-WB-09 | Empty `customerID` rejected | CC | PASS |
| TC-WB-10 | `customerID` length = 2 rejected | CC | PASS |
| TC-WB-11 | `@` in `customerID` should throw | CC | **FAIL** (known defect) |
| TC-WB-12 | Invalid category `Hardware` rejected | EC | PASS |
| TC-WB-13 | Agent assigned by matching skill; `openTickets++` | SC | PASS |
| TC-WB-14 | Fallback to `Other`-skill agent when no match | BC | PASS |
| TC-WB-15 | 4-char description rejected (min 10) | EC | PASS |

### Black Box Tests (15 cases — `registerComplaint`, ECP & BVA)

| ID | Description | Technique | Result |
|---|---|---|---|
| TC-BB-01 | Valid: `Billing`/`High` — ticket generated, status `Open` | ECP | PASS |
| TC-BB-02 | Valid: `Critical` — SLA = 4 h | ECP | PASS |
| TC-BB-03 | Valid: `Low` — SLA = 72 h | ECP | PASS |
| TC-BB-04 | Valid: `Medium`/`Service` — SLA = 48 h | ECP | PASS |
| TC-BB-05 | All 5 categories accepted | ECP | PASS |
| TC-BB-06 | Empty `customerID` rejected | ECP | PASS |
| TC-BB-07 | Invalid category `Hardware` rejected | ECP | PASS |
| TC-BB-08 | Invalid priority `Urgent` rejected | ECP | PASS |
| TC-BB-09 | Description = 9 chars should be rejected | BVA | **FAIL** (known defect) |
| TC-BB-10 | Description = 501 chars rejected | BVA | PASS |
| TC-BB-11 | `customerID` length = 4 (min boundary) accepted | BVA | PASS |
| TC-BB-12 | `customerID` length = 3 (below min) rejected | BVA | PASS |
| TC-BB-13 | `customerID` length = 12 (max boundary) accepted | BVA | PASS |
| TC-BB-14 | Description length = 10 (exact min) accepted | BVA | PASS |
| TC-BB-15 | Load balancing — least-loaded Billing agent assigned | BVA | PASS |

### Summary

| Metric | Value |
|---|---|
| Total Tests | 30 |
| Passed | 27 |
| Failed | 3 |
| Pass Rate | **90.0%** |

---

## Code Coverage Report

The implementation includes a `CoverageTracker` struct that instruments 26 named branches across all key methods and prints a branch-coverage report at the end of each run.

| Branch | Status |
|---|---|
| `validateCustomerID:entry` | HIT |
| `validateCustomerID:empty` | HIT |
| `validateCustomerID:bad_length` | HIT |
| `validateCustomerID:length_ok` | HIT |
| `validateCustomerID:non_alnum_found` | HIT |
| `validateCategory:entry` | HIT |
| `validateCategory:valid` | HIT |
| `validateCategory:invalid` | HIT |
| `validateDescription:entry` | HIT |
| `validateDescription:empty` | MISS |
| `validateDescription:too_short` | HIT |
| `validateDescription:length_ok` | HIT |
| `validateDescription:too_long` | HIT |
| `validatePriority:entry` | HIT |
| `validatePriority:valid` | HIT |
| `validatePriority:invalid` | HIT |
| `assignAgent:skill_match` | HIT |
| `assignAgent:fallback_to_other` | MISS |
| `assignAgent:no_agent_found` | MISS |
| `escalateComplaint:entry` | HIT |
| `escalateComplaint:not_found` | HIT |
| `escalateComplaint:closed_guard` | HIT |
| `escalateComplaint:already_escalated` | HIT |
| `escalateComplaint:high_or_critical` | HIT |
| `escalateComplaint:medium_promoted` | HIT |
| `escalateComplaint:low_not_promoted` | HIT |

**Branches Hit: 23 / 26 — Coverage: 88.5%**

The 3 missed branches (`validateDescription:empty`, `assignAgent:fallback_to_other`, `assignAgent:no_agent_found`) indicate paths not triggered by the current test suite and represent opportunities for additional test cases.

---

## Build & Run

```bash
# Compile
g++ -std=c++17 -o output implementation.cpp

# Run
./output
```

No external dependencies are required. The program prints the full test suite output followed by the test summary and code coverage report to stdout.

---

## Agent Pool (Seed Data)

| Agent ID | Name | Skill |
|---|---|---|
| A001 | Rahul Sharma | Billing |
| A002 | Priya Mehta | Network |
| A003 | Amit Joshi | Technical |
| A004 | Sunita Rao | Service |
| A005 | Vikram Nair | Other |
| A006 | Neha Gupta | Billing *(extra for load-balancing tests)* |
