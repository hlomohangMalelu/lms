# Self-code-review checklist

Review the checklist for each feature branch before merging. Record `Pass`, `Fail` or `N/A` and link the corrective commit for every failure.

| Area | Review question | Result |
|---|---|---|
| Requirements | Does the change implement a stated requirement and preserve Phase 3 design intent? | Pending per change |
| Interfaces | Are public interfaces minimal and do implementations satisfy their contracts? | Pending per change |
| SOLID/design | Are responsibilities cohesive and dependencies injected rather than hidden? | Pending per change |
| Validation | Are inputs and boundary conditions validated? | Pending per change |
| Security | Are permissions checked server/service-side and SQL values parameterized? | Pending per change |
| Persistence | Are related writes transactional and errors handled without partial state? | Pending per change |
| Tests | Are normal, boundary, negative and exception paths tested? | Pending per change |
| Portability | Does code avoid OS-specific paths, shell assumptions and compiler extensions? | Pending per change |
| Diagnostics | Are warnings addressed and are errors useful to the user? | Pending per change |
| Documentation | Are README, test evidence, traceability and defect records updated? | Pending per change |
| Hygiene | Are secrets, databases, build outputs and generated coverage files excluded? | Pending per change |

## Review record template

- Branch / commit:
- Reviewer:
- Date:
- Checks failed:
- Changes made as a result:
- Build and test command:
- Test result:
- Merge decision:
