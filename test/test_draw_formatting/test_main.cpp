#include <unity.h>
#include <cstring>
#include "draw_formatting.h"

void setUp() {}
void tearDown() {}

// --- scoreToDigits ---

void test_score_zero() {
    ScoreDigits d = scoreToDigits(0);
    TEST_ASSERT_EQUAL_INT(0, d.hundreds);
    TEST_ASSERT_EQUAL_INT(0, d.tens);
    TEST_ASSERT_EQUAL_INT(0, d.ones);
    TEST_ASSERT_FALSE(d.hasHundreds);
}

void test_score_single_digit() {
    ScoreDigits d = scoreToDigits(9);
    TEST_ASSERT_EQUAL_INT(0, d.tens);
    TEST_ASSERT_EQUAL_INT(9, d.ones);
    TEST_ASSERT_FALSE(d.hasHundreds);
}

void test_score_two_digit_boundary_10() {
    ScoreDigits d = scoreToDigits(10);
    TEST_ASSERT_EQUAL_INT(1, d.tens);
    TEST_ASSERT_EQUAL_INT(0, d.ones);
    TEST_ASSERT_FALSE(d.hasHundreds);
}

void test_score_two_digit_99() {
    ScoreDigits d = scoreToDigits(99);
    TEST_ASSERT_EQUAL_INT(9, d.tens);
    TEST_ASSERT_EQUAL_INT(9, d.ones);
    TEST_ASSERT_FALSE(d.hasHundreds);
}

void test_score_three_digit_boundary_100() {
    ScoreDigits d = scoreToDigits(100);
    TEST_ASSERT_EQUAL_INT(1, d.hundreds);
    TEST_ASSERT_EQUAL_INT(0, d.tens);
    TEST_ASSERT_EQUAL_INT(0, d.ones);
    TEST_ASSERT_TRUE(d.hasHundreds);
}

void test_score_three_digit_999() {
    ScoreDigits d = scoreToDigits(999);
    TEST_ASSERT_EQUAL_INT(9, d.hundreds);
    TEST_ASSERT_EQUAL_INT(9, d.tens);
    TEST_ASSERT_EQUAL_INT(9, d.ones);
    TEST_ASSERT_TRUE(d.hasHundreds);
}

// --- quarterLabel ---

void test_quarter_1st() { TEST_ASSERT_EQUAL_STRING("1st", quarterLabel(1)); }
void test_quarter_2nd() { TEST_ASSERT_EQUAL_STRING("2nd", quarterLabel(2)); }
void test_quarter_3rd() { TEST_ASSERT_EQUAL_STRING("3rd", quarterLabel(3)); }
void test_quarter_4th() { TEST_ASSERT_EQUAL_STRING("4th", quarterLabel(4)); }
void test_quarter_half() { TEST_ASSERT_EQUAL_STRING("HT", quarterLabel(-1)); }
void test_quarter_first_overtime() { TEST_ASSERT_EQUAL_STRING("OT", quarterLabel(5)); }
void test_quarter_second_overtime() { TEST_ASSERT_EQUAL_STRING("OT2", quarterLabel(6)); }
void test_quarter_third_overtime() { TEST_ASSERT_EQUAL_STRING("OT3", quarterLabel(7)); }
void test_quarter_many_overtimes_wraps_mod_10() {
    // period 15 -> otNum = 11 -> "OT1" (11 % 10)
    TEST_ASSERT_EQUAL_STRING("OT1", quarterLabel(15));
}
void test_quarter_zero_is_unrecognized() { TEST_ASSERT_EQUAL_STRING("", quarterLabel(0)); }
void test_quarter_negative_other_than_half_is_unrecognized() {
    TEST_ASSERT_EQUAL_STRING("", quarterLabel(-2));
}

// --- parseMMSS ---

void test_parse_mmss_valid() {
    int m, s;
    TEST_ASSERT_TRUE(parseMMSS("12:34", m, s));
    TEST_ASSERT_EQUAL_INT(12, m);
    TEST_ASSERT_EQUAL_INT(34, s);
}

void test_parse_mmss_leading_zero_minutes() {
    int m, s;
    TEST_ASSERT_TRUE(parseMMSS("00:05", m, s));
    TEST_ASSERT_EQUAL_INT(0, m);
    TEST_ASSERT_EQUAL_INT(5, s);
}

void test_parse_mmss_empty_string_fails() {
    int m, s;
    TEST_ASSERT_FALSE(parseMMSS("", m, s));
}

void test_parse_mmss_null_fails() {
    int m, s;
    TEST_ASSERT_FALSE(parseMMSS(nullptr, m, s));
}

void test_parse_mmss_no_colon_fails() {
    int m, s;
    TEST_ASSERT_FALSE(parseMMSS("1234", m, s));
}

// --- parseScheduleTime ---

void test_parse_schedule_time_strips_et_suffix() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 PM ET", h, m, pm));
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(30, m);
    TEST_ASSERT_TRUE(pm);
}

void test_parse_schedule_time_strips_est_suffix() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 PM EST", h, m, pm));
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(30, m);
    TEST_ASSERT_TRUE(pm);
}

void test_parse_schedule_time_strips_edt_suffix() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 PM EDT", h, m, pm));
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(30, m);
    TEST_ASSERT_TRUE(pm);
}

void test_parse_schedule_time_no_suffix_unaffected() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 PM", h, m, pm));
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(30, m);
    TEST_ASSERT_TRUE(pm);
}

void test_parse_schedule_time_lowercase_pm_detected() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 pm", h, m, pm));
    TEST_ASSERT_TRUE(pm);
}

void test_parse_schedule_time_am_defaults_false() {
    int h, m; bool pm;
    TEST_ASSERT_TRUE(parseScheduleTime("9:15 AM", h, m, pm));
    TEST_ASSERT_FALSE(pm);
}

void test_parse_schedule_time_long_input_truncated_safely() {
    int h, m; bool pm;
    // 24-byte buf in the implementation - this input is deliberately longer.
    TEST_ASSERT_TRUE(parseScheduleTime("7:30 PM ET (subject to change)", h, m, pm));
    TEST_ASSERT_EQUAL_INT(7, h);
}

void test_parse_schedule_time_no_colon_fails() {
    int h, m; bool pm;
    TEST_ASSERT_FALSE(parseScheduleTime("Final", h, m, pm));
}

void test_parse_schedule_time_empty_fails() {
    int h, m; bool pm;
    TEST_ASSERT_FALSE(parseScheduleTime("", h, m, pm));
}

// --- parseDateYMD ---

void test_parse_date_well_formed() {
    int y, mo, d;
    TEST_ASSERT_TRUE(parseDateYMD("2026-07-04", y, mo, d));
    TEST_ASSERT_EQUAL_INT(2026, y);
    TEST_ASSERT_EQUAL_INT(7, mo);
    TEST_ASSERT_EQUAL_INT(4, d);
}

void test_parse_date_malformed_fails() {
    int y, mo, d;
    TEST_ASSERT_FALSE(parseDateYMD("not-a-date", y, mo, d));
}

void test_parse_date_null_fails() {
    int y, mo, d;
    TEST_ASSERT_FALSE(parseDateYMD(nullptr, y, mo, d));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_score_zero);
    RUN_TEST(test_score_single_digit);
    RUN_TEST(test_score_two_digit_boundary_10);
    RUN_TEST(test_score_two_digit_99);
    RUN_TEST(test_score_three_digit_boundary_100);
    RUN_TEST(test_score_three_digit_999);

    RUN_TEST(test_quarter_1st);
    RUN_TEST(test_quarter_2nd);
    RUN_TEST(test_quarter_3rd);
    RUN_TEST(test_quarter_4th);
    RUN_TEST(test_quarter_half);
    RUN_TEST(test_quarter_first_overtime);
    RUN_TEST(test_quarter_second_overtime);
    RUN_TEST(test_quarter_third_overtime);
    RUN_TEST(test_quarter_many_overtimes_wraps_mod_10);
    RUN_TEST(test_quarter_zero_is_unrecognized);
    RUN_TEST(test_quarter_negative_other_than_half_is_unrecognized);

    RUN_TEST(test_parse_mmss_valid);
    RUN_TEST(test_parse_mmss_leading_zero_minutes);
    RUN_TEST(test_parse_mmss_empty_string_fails);
    RUN_TEST(test_parse_mmss_null_fails);
    RUN_TEST(test_parse_mmss_no_colon_fails);

    RUN_TEST(test_parse_schedule_time_strips_et_suffix);
    RUN_TEST(test_parse_schedule_time_strips_est_suffix);
    RUN_TEST(test_parse_schedule_time_strips_edt_suffix);
    RUN_TEST(test_parse_schedule_time_no_suffix_unaffected);
    RUN_TEST(test_parse_schedule_time_lowercase_pm_detected);
    RUN_TEST(test_parse_schedule_time_am_defaults_false);
    RUN_TEST(test_parse_schedule_time_long_input_truncated_safely);
    RUN_TEST(test_parse_schedule_time_no_colon_fails);
    RUN_TEST(test_parse_schedule_time_empty_fails);

    RUN_TEST(test_parse_date_well_formed);
    RUN_TEST(test_parse_date_malformed_fails);
    RUN_TEST(test_parse_date_null_fails);

    return UNITY_END();
}
