#include <unity.h>
#include "clock_sync.h"

void setUp() {}
void tearDown() {}

// Regression tests for the clock catch-up fix (bug 1). nextDisplaySeconds()
// is the exact decision pollLiveGame() makes each poll - pulled out into a
// pure function so it's testable without a live game feed.

void test_first_tick_seeds_from_negative() {
    // displaySecs_ starts at -1 before the first successful poll.
    TEST_ASSERT_EQUAL_INT(750, nextDisplaySeconds(-1, 720, 30));
}

void test_normal_decrement_does_not_reseed() {
    // apiSecs at or below the buffered displaySecs -> no reseed, caller's
    // existing decrement loop keeps counting down toward it.
    TEST_ASSERT_EQUAL_INT(700, nextDisplaySeconds(700, 690, 30));
}

void test_normal_decrement_equal_values_does_not_reseed() {
    // Buffer fully caught up (displaySecs == target) -> holds steady.
    TEST_ASSERT_EQUAL_INT(690, nextDisplaySeconds(690, 690, 30));
}

void test_period_transition_jump_reseeds() {
    // THE BUG: Q1 ends near 0:00, Q2 starts at 12:00 (720s). Before the fix,
    // this returned the stale low value (near 0) forever.
    TEST_ASSERT_EQUAL_INT(750, nextDisplaySeconds(2, 720, 30));
}

void test_overtime_transition_jump_reseeds() {
    // Q4 ends near 0:00, OT starts at 5:00 (300s in NBA).
    TEST_ASSERT_EQUAL_INT(330, nextDisplaySeconds(0, 300, 30));
}

void test_small_jitter_just_above_does_reseed() {
    // Not a period transition, just apiSecs ticking ahead of a
    // fully-caught-up display by a couple seconds - still correctly
    // triggers a (small) reseed per the current apiSecs > displaySecs rule.
    TEST_ASSERT_EQUAL_INT(32, nextDisplaySeconds(0, 2, 30));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_first_tick_seeds_from_negative);
    RUN_TEST(test_normal_decrement_does_not_reseed);
    RUN_TEST(test_normal_decrement_equal_values_does_not_reseed);
    RUN_TEST(test_period_transition_jump_reseeds);
    RUN_TEST(test_overtime_transition_jump_reseeds);
    RUN_TEST(test_small_jitter_just_above_does_reseed);
    return UNITY_END();
}
