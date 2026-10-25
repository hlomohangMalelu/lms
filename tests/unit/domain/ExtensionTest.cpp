#include <gtest/gtest.h>

#include "domain/extension/Extension.h"

using namespace lms::domain;

TEST(ExtensionTest, StoresExtensionAndRelatedIdentifiers) {
    Extension extension(
        1,
        10,
        20,
        5,
        "2026-12-05 23:59:00",
        "Student was granted additional time."
    );

    EXPECT_EQ(extension.extensionId(), 1);
    EXPECT_EQ(extension.assignmentId(), 10);
    EXPECT_EQ(extension.studentId(), 20);
    EXPECT_EQ(extension.grantedBy(), 5);
}

TEST(ExtensionTest, StoresNewDeadline) {
    Extension extension(
        1,
        10,
        20,
        5,
        "2026-12-05 23:59:00",
        "Student was granted additional time."
    );

    EXPECT_EQ(
        extension.newDeadline(),
        "2026-12-05 23:59:00"
    );
}

TEST(ExtensionTest, StoresReason) {
    Extension extension(
        1,
        10,
        20,
        5,
        "2026-12-05 23:59:00",
        "Student was granted additional time."
    );

    EXPECT_EQ(
        extension.reason(),
        "Student was granted additional time."
    );
}
