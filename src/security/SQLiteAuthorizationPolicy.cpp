#include "security/SQLiteAuthorizationPolicy.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace lms::security {

    namespace {

        bool exists(
            SQLite::Database& db,
            const std::string& sql,
            long long a,
            long long b = 0,
            long long c = 0
        ) {
            SQLite::Statement query(db, sql);

            query.bind(1, static_cast<std::int64_t>(a));

            if (sql.find("?") != std::string::npos &&
                sql.find("?") != sql.rfind("?")) {
                query.bind(2, static_cast<std::int64_t>(b));
            }

            if (std::count(sql.begin(), sql.end(), '?') > 2) {
                query.bind(3, static_cast<std::int64_t>(c));
            }

            return query.executeStep();
        }

    } // namespace

    bool SQLiteAuthorizationPolicy::isActiveUser(
        long long id,
        const char* role
    ) const {
        SQLite::Statement query(
            database_,
            "SELECT 1 FROM users WHERE id=? AND role=? AND active=1"
        );

        query.bind(1, static_cast<std::int64_t>(id));
        query.bind(2, role);

        return query.executeStep();
    }

    bool SQLiteAuthorizationPolicy::isAdministrator(long long id) const {
        return isActiveUser(id, "ADMIN");
    }

    bool SQLiteAuthorizationPolicy::canSubmitAssignment(
        long long id,
        long long assignment
    ) const {
        return exists(
            database_,
            "SELECT 1 FROM users u "
            "JOIN students s ON s.user_id=u.id "
            "JOIN enrolments e ON e.student_id=s.user_id "
            "JOIN assignments a ON a.course_id=e.course_id "
            "WHERE u.id=? AND a.id=? "
            "AND u.role='STUDENT' AND u.active=1",
            id,
            assignment
        );
    }

    bool SQLiteAuthorizationPolicy::canGrantExtension(
        long long lecturer,
        long long assignment,
        long long student
    ) const {
        return exists(
            database_,
            "SELECT 1 FROM users u "
            "JOIN lecturers l ON l.user_id=u.id "
            "JOIN course_lecturers cl ON cl.lecturer_id=l.user_id "
            "JOIN assignments a ON a.course_id=cl.course_id "
            "JOIN enrolments e ON e.course_id=a.course_id "
            "WHERE u.id=? AND a.id=? AND e.student_id=? "
            "AND u.role='LECTURER' AND u.active=1",
            lecturer,
            assignment,
            student
        );
    }

    bool SQLiteAuthorizationPolicy::canMarkSubmission(
        long long lecturer,
        long long submission
    ) const {
        return exists(
            database_,
            "SELECT 1 FROM users u "
            "JOIN lecturers l ON l.user_id=u.id "
            "JOIN course_lecturers cl ON cl.lecturer_id=l.user_id "
            "JOIN assignments a ON a.course_id=cl.course_id "
            "JOIN submissions s ON s.assignment_id=a.id "
            "WHERE u.id=? AND s.id=? "
            "AND u.role='LECTURER' AND u.active=1",
            lecturer,
            submission
        );
    }

    bool SQLiteAuthorizationPolicy::canModerateSubmission(
        long long hod,
        long long submission
    ) const {
        return exists(
            database_,
            "SELECT 1 FROM users u "
            "JOIN heads_of_department h ON h.user_id=u.id "
            "JOIN courses c ON c.department_id=h.department_id "
            "JOIN assignments a ON a.course_id=c.id "
            "JOIN submissions s ON s.assignment_id=a.id "
            "WHERE u.id=? AND s.id=? "
            "AND u.role='HOD' AND u.active=1",
            hod,
            submission
        );
    }

    void SQLiteAuthorizationPolicy::requireActiveRole(
        long long id,
        const char* role
    ) const {
        if (!isActiveUser(id, role)) {
            throw std::runtime_error(
                "Access denied: active " + std::string(role) +
                " account required."
            );
        }
    }

    void SQLiteAuthorizationPolicy::requireAdministrator(long long id) const {
        if (!isAdministrator(id)) {
            throw std::runtime_error(
                "Access denied: active administrator account required."
            );
        }
    }

    void SQLiteAuthorizationPolicy::requireCanSubmit(
        long long id,
        long long assignment
    ) const {
        if (!canSubmitAssignment(id, assignment)) {
            throw std::runtime_error(
                "Access denied: student is not enrolled in this "
                "assignment's course."
            );
        }
    }

    void SQLiteAuthorizationPolicy::requireCanGrantExtension(
        long long id,
        long long assignment,
        long long student
    ) const {
        if (!canGrantExtension(id, assignment, student)) {
            throw std::runtime_error(
                "Access denied: lecturer is not assigned to this course "
                "or student is not enrolled."
            );
        }
    }

    void SQLiteAuthorizationPolicy::requireCanMark(
        long long id,
        long long submission
    ) const {
        if (!canMarkSubmission(id, submission)) {
            throw std::runtime_error(
                "Access denied: lecturer is not assigned to this "
                "submission's course."
            );
        }
    }

    void SQLiteAuthorizationPolicy::requireCanModerate(
        long long id,
        long long submission
    ) const {
        if (!canModerateSubmission(id, submission)) {
            throw std::runtime_error(
                "Access denied: HoD is not responsible for this department."
            );
        }
    }

} // namespace lms::security