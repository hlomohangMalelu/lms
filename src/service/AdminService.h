#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <string>
#include "security/IAuthorizationPolicy.h"

namespace lms::service {

    class AdminService {
        public:
            AdminService(SQLite::Database& database, const lms::security::IAuthorizationPolicy& authorization, long long actorId);

            long long createDepartment(
                const std::string& name
            );

            long long createCourse(
                const std::string& code,
                const std::string& title,
                long long departmentId
            );

            long long createUser(
                const std::string& username,
                const std::string& passwordHash,
                const std::string& fullName,
                const std::string& role
            );

            void deactivateUser(
                long long userId
            );

            void registerStudent(
                long long userId,
                const std::string& studentNumber
            );

            void registerLecturer(
                long long userId,
                const std::string& staffNumber
            );

            void assignHeadOfDepartment(
                long long userId,
                long long departmentId
            );

            void assignLecturerToCourse(
                long long lecturerId,
                long long courseId
            );

            void enrolStudent(
                long long studentId,
                long long courseId
            );

        private:
            SQLite::Database& database_;
            const lms::security::IAuthorizationPolicy& authorization_;
            long long actorId_;
    };

} // namespace lms::service
