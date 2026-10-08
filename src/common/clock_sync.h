/*
********************************************************************************

    The clock catch-up decision. Fixes a real bug: displaySecs_ used to
    freeze instead of jumping forward at period/OT transitions.

********************************************************************************
*/

#pragma once

int nextDisplaySeconds(int displaySecs, int apiSecs, int delaySecs);
