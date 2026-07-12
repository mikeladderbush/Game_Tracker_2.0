#include <unity.h>
#include "api_formatting.h"

void setUp() {}
void tearDown() {}

// --- clockStrToSecs ---

void test_pt_format_full() {
    TEST_ASSERT_EQUAL_INT(720, clockStrToSecs("PT12M00.00S"));
}

void test_pt_format_zero() {
    TEST_ASSERT_EQUAL_INT(0, clockStrToSecs("PT0M00.00S"));
}

void test_pt_format_single_digit_decimal_seconds() {
    TEST_ASSERT_EQUAL_INT(309, clockStrToSecs("PT5M9.30S"));
}

void test_clock_empty_string_returns_negative_one() {
    TEST_ASSERT_EQUAL_INT(-1, clockStrToSecs(""));
}

void test_pt_alone_no_minutes_or_seconds() {
    // Falls through with minutes=0, seconds=0 rather than -1. Documenting
    // the current behavior; flag if this should actually be treated as
    // "unparseable" instead.
    TEST_ASSERT_EQUAL_INT(0, clockStrToSecs("PT"));
}

void test_mmss_format_no_decimal() {
    TEST_ASSERT_EQUAL_INT(754, clockStrToSecs("12:34"));
}

void test_mmss_format_decimal_seconds() {
    TEST_ASSERT_EQUAL_INT(754, clockStrToSecs("12:34.5"));
}

void test_mmss_with_trailing_text_after_space() {
    TEST_ASSERT_EQUAL_INT(309, clockStrToSecs("5:09 PM"));
}

void test_final_string_no_colon_no_pt_returns_negative_one() {
    TEST_ASSERT_EQUAL_INT(-1, clockStrToSecs("Final"));
}

void test_garbage_string_returns_negative_one() {
    TEST_ASSERT_EQUAL_INT(-1, clockStrToSecs("abc"));
}

// --- secsToMMSS ---

void test_secs_zero() {
    TEST_ASSERT_EQUAL_STRING("00:00", secsToMMSS(0).c_str());
}

void test_secs_negative_clamped_to_zero() {
    TEST_ASSERT_EQUAL_STRING("00:00", secsToMMSS(-5).c_str());
}

void test_secs_under_a_minute() {
    TEST_ASSERT_EQUAL_STRING("00:59", secsToMMSS(59).c_str());
}

void test_secs_exactly_a_minute() {
    TEST_ASSERT_EQUAL_STRING("01:00", secsToMMSS(60).c_str());
}

void test_secs_near_end_of_quarter() {
    TEST_ASSERT_EQUAL_STRING("11:59", secsToMMSS(719).c_str());
}

void test_secs_over_99_minutes_still_fits_buffer() {
    TEST_ASSERT_EQUAL_STRING("100:00", secsToMMSS(6000).c_str());
}

// --- convertUtcToEstWithOffset ---
// convertUtcToEst() (api_client.cpp) reads the DST offset from the live
// synced clock via isDaylightSavingNow(), which needs a real RTC and can't
// run natively - that seasonal flip is a manual/on-device check. This tests
// the conversion math itself (wraparound, AM/PM, formatting) directly, with
// both possible offsets passed in explicitly.

void test_utc_to_est_no_wraparound_est() {
    // "00:30" UTC, EST (-5) -> 19:30 previous day -> "7:30 PM"
    TEST_ASSERT_EQUAL_STRING("7:30 PM", convertUtcToEstWithOffset("00:30", -5).c_str());
}

void test_utc_to_est_no_wraparound_edt() {
    // "00:30" UTC, EDT (-4) -> 20:30 previous day -> "8:30 PM"
    TEST_ASSERT_EQUAL_STRING("8:30 PM", convertUtcToEstWithOffset("00:30", -4).c_str());
}

void test_utc_to_est_day_rollover_hour() {
    // "23:59" UTC, EST (-5) -> 18:59 same day -> "6:59 PM"
    TEST_ASSERT_EQUAL_STRING("6:59 PM", convertUtcToEstWithOffset("23:59", -5).c_str());
}

void test_utc_to_est_noon_boundary() {
    TEST_ASSERT_EQUAL_STRING("7:00 AM", convertUtcToEstWithOffset("12:00", -5).c_str());
}

void test_utc_to_est_midnight_boundary_hours12_wraps_to_12() {
    // "05:00" UTC, EST (-5) -> 00:00 -> hours12 should read 12, not 0.
    TEST_ASSERT_EQUAL_STRING("12:00 AM", convertUtcToEstWithOffset("05:00", -5).c_str());
}

void test_utc_to_est_malformed_no_colon_passthrough() {
    TEST_ASSERT_EQUAL_STRING("garbage", convertUtcToEstWithOffset("garbage", -5).c_str());
}

void test_utc_to_est_empty_passthrough() {
    TEST_ASSERT_EQUAL_STRING("", convertUtcToEstWithOffset("", -5).c_str());
}

// --- teamNameToId ---

void test_team_name_to_id_known_team() {
    TEST_ASSERT_EQUAL_INT(14, teamNameToId("Lakers"));
}

void test_team_name_to_id_unknown_team() {
    TEST_ASSERT_EQUAL_INT(-1, teamNameToId("Fakers"));
}

void test_team_name_to_id_is_case_sensitive() {
    // Documents current behavior (raw strcmp) - callers must pass canonical
    // case. Flag if this should become case-insensitive.
    TEST_ASSERT_EQUAL_INT(-1, teamNameToId("lakers"));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_pt_format_full);
    RUN_TEST(test_pt_format_zero);
    RUN_TEST(test_pt_format_single_digit_decimal_seconds);
    RUN_TEST(test_clock_empty_string_returns_negative_one);
    RUN_TEST(test_pt_alone_no_minutes_or_seconds);
    RUN_TEST(test_mmss_format_no_decimal);
    RUN_TEST(test_mmss_format_decimal_seconds);
    RUN_TEST(test_mmss_with_trailing_text_after_space);
    RUN_TEST(test_final_string_no_colon_no_pt_returns_negative_one);
    RUN_TEST(test_garbage_string_returns_negative_one);

    RUN_TEST(test_secs_zero);
    RUN_TEST(test_secs_negative_clamped_to_zero);
    RUN_TEST(test_secs_under_a_minute);
    RUN_TEST(test_secs_exactly_a_minute);
    RUN_TEST(test_secs_near_end_of_quarter);
    RUN_TEST(test_secs_over_99_minutes_still_fits_buffer);

    RUN_TEST(test_utc_to_est_no_wraparound_est);
    RUN_TEST(test_utc_to_est_no_wraparound_edt);
    RUN_TEST(test_utc_to_est_day_rollover_hour);
    RUN_TEST(test_utc_to_est_noon_boundary);
    RUN_TEST(test_utc_to_est_midnight_boundary_hours12_wraps_to_12);
    RUN_TEST(test_utc_to_est_malformed_no_colon_passthrough);
    RUN_TEST(test_utc_to_est_empty_passthrough);

    RUN_TEST(test_team_name_to_id_known_team);
    RUN_TEST(test_team_name_to_id_unknown_team);
    RUN_TEST(test_team_name_to_id_is_case_sensitive);

    return UNITY_END();
}
