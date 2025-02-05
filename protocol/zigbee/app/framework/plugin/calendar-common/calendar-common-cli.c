/***************************************************************************//**
 * @file
 * @brief Routines for the Calendar Common plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "calendar-common.h"

//-----------------------------------------------------------------------------
// Globals

uint8_t calendarName0[SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1] =
{ 0x0C, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
uint8_t calendarName1[SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1] =
{ 0x0C, 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x' };

const char* calendarTypeStrings[] = {
  "Delivered",
  "Received",
  "Delivered and Received",
  "Friendly Credit",
  "Auxilliary Load Switch",
};
#define MAX_CALENDAR_TYPE SL_ZIGBEE_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR

#define PROVIDER_ID_START     0x0000A000
#define ISSUER_EVENT_ID_START 0x0000B000
#define CALENDAR_ID_START     0

#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY (MINUTES_PER_HOUR * 24)

#define SCHEDULE_ENTRIES_DATA_OFFSET 0xC0

static uint8_t calendarIndexForPrinting = 0;

//-----------------------------------------------------------------------------

static bool printSelectedCalendar(void)
{
  sl_zigbee_af_calendar_cluster_println("Selected Calendar Index: %d",
                                        calendarIndexForPrinting);
  if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= calendarIndexForPrinting
      || calendars[calendarIndexForPrinting].calendarId == SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID) {
    sl_zigbee_af_calendar_cluster_println("No data.");
    return false;
  }
  return true;
}

static void printScheduleEntryData(uint8_t calendarType, uint8_t data)
{
  switch (calendarType) {
    case SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR:
    case SL_ZIGBEE_ZCL_CALENDAR_TYPE_RECEIVED_CALENDAR:
    case SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_AND_RECEIVED_CALENDAR:
      sl_zigbee_af_calendar_cluster_print("Price Tier: 0x%X", data);
      break;
    case SL_ZIGBEE_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR:
      sl_zigbee_af_calendar_cluster_print("Friendly credit: %p",
                                          (data == 0
                                           ? "not available"
                                           : "enabled"));
      break;
    case SL_ZIGBEE_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR:
      sl_zigbee_af_calendar_cluster_print("Auxilliary Switch state: 0x%X", data);
      break;
    default:
      sl_zigbee_af_calendar_cluster_print("Invalid calendar type (%d), data: %d",
                                          calendarType,
                                          data);
      break;
  }
}

static void printTime(const char* prefix,
                      uint32_t zigbeeUtcTime)
{
  sl_zigbee_af_time_struct_t time;
  sl_zigbee_af_fill_time_struct_from_utc(zigbeeUtcTime, &time);
  sl_zigbee_af_calendar_cluster_print("%p", prefix);
  sl_zigbee_af_calendar_cluster_println("%4X (%d/%p%d/%p%d, %p%d:%p%d:%p%d)",
                                        zigbeeUtcTime,
                                        time.year,
                                        (time.month < 10 ? "0" : ""),
                                        time.month,
                                        (time.day < 10 ? "0" : ""),
                                        time.day,
                                        (time.hours < 10 ? "0" : ""),
                                        time.hours,
                                        (time.minutes < 10 ? "0" : ""),
                                        time.minutes,
                                        (time.seconds < 10 ? "0" : ""),
                                        time.seconds);
}

static void printScheduleEntry(const sl_zigbee_af_calendar_day_schedule_entry_struct_t* scheduleEntry,
                               uint8_t calendarType)
{
  uint8_t hours = 0;
  uint8_t minutes = 0;
  if (scheduleEntry->minutesFromMidnight != 0) {
    hours = scheduleEntry->minutesFromMidnight / MINUTES_PER_HOUR;
    minutes = scheduleEntry->minutesFromMidnight % MINUTES_PER_HOUR;
  }
  sl_zigbee_af_calendar_cluster_print("%s%d:%s%d - ",
                                      (hours < 10 ? "0" : ""),
                                      hours,
                                      (minutes < 10 ? "0" : ""),
                                      minutes);
  printScheduleEntryData(calendarType, scheduleEntry->data);
  sl_zigbee_af_calendar_cluster_println("");
}

static void printDayProfile(const sl_zigbee_af_calendar_day_struct_t* day,
                            uint8_t calendarType)
{
  uint8_t i;
  for (i = 0; i < day->numberOfScheduleEntries; i++) {
    sl_zigbee_af_calendar_cluster_print("  ");
    printScheduleEntry(&(day->scheduleEntries[i]),
                       calendarType);
  }
}

static void printSpecialDayProfile(const sl_zigbee_af_calendar_special_day_struct_t* specialDay,
                                   const sl_zigbee_af_calendar_day_struct_t* normalDays)
{
  sl_zigbee_af_calendar_cluster_print("  Date: ");
  sl_zigbee_af_print_dateln(&specialDay->startDate);
  sl_zigbee_af_calendar_cluster_println("  Day ID: %d", normalDays[specialDay->normalDayIndex].id);
}

static void printDayProfiles(const sl_zigbee_af_calendar_struct_t* calendar,
                             bool printSpecialDays)
{
  uint8_t i;
  uint8_t max = (printSpecialDays
                 ? calendar->numberOfSpecialDayProfiles
                 : calendar->numberOfDayProfiles);
  for (i = 0; i < max; i++) {
    if (printSpecialDays) {
      sl_zigbee_af_calendar_cluster_println("Special Day: %d", i);
      printSpecialDayProfile(&(calendar->specialDays[i]),
                             calendar->normalDays);
    } else {
      sl_zigbee_af_calendar_cluster_println("Day ID: %d - Schedule",
                                            calendar->normalDays[i].id);
      printDayProfile(&(calendar->normalDays[i]),
                      calendar->calendarType);
    }
  }
}

static void printCalendar(const sl_zigbee_af_calendar_struct_t* calendar)
{
  sl_zigbee_af_time_struct_t time;
  sl_zigbee_af_fill_time_struct_from_utc(calendar->startTimeUtc, &time);
  sl_zigbee_af_calendar_cluster_print("Name: ");
  sl_zigbee_af_calendar_cluster_print_string(calendar->name);
  sl_zigbee_af_calendar_cluster_println("");
  sl_zigbee_af_calendar_cluster_println("ID:              0x%4X", calendar->calendarId);
  sl_zigbee_af_calendar_cluster_println("Issuer Event ID: 0x%4X", calendar->issuerEventId);
  sl_zigbee_af_calendar_cluster_println("Provider ID:     0x%4X", calendar->providerId);
  sl_zigbee_af_calendar_cluster_println("Type:            %d (%p)",
                                        calendar->calendarType,
                                        (calendar->calendarType <= MAX_CALENDAR_TYPE
                                         ? calendarTypeStrings[calendar->calendarType]
                                         : "???"));
  printTime("Start Time:      ", calendar->startTimeUtc);
  sl_zigbee_af_calendar_cluster_println("Season Profiles:      %d", calendar->numberOfSeasons);
  sl_zigbee_af_calendar_cluster_println("Week Profiles:        %d", calendar->numberOfWeekProfiles);
  sl_zigbee_af_calendar_cluster_println("Day Profiles:         %d", calendar->numberOfDayProfiles);
  sl_zigbee_af_calendar_cluster_println("Special Day Profiles: %d", calendar->numberOfSpecialDayProfiles);
}

// The offset is useful for making each day's schedule entry different so as it tell
// the days apart.
static void setupTestScheduleEntries(sl_zigbee_af_calendar_day_struct_t* day, uint8_t offset)
{
  uint8_t i;
  for (i = 0; i < day->numberOfScheduleEntries; i++) {
    uint16_t temp = (i * MINUTES_PER_HOUR) + offset;
    if (temp >= MINUTES_PER_DAY) {
      temp = MINUTES_PER_DAY - 1;
    }
    day->scheduleEntries[i].minutesFromMidnight = temp;
    day->scheduleEntries[i].data = SCHEDULE_ENTRIES_DATA_OFFSET + i;
  }
}

static void setupTestDays(sl_zigbee_af_calendar_struct_t* calendar, uint8_t offset)
{
  uint8_t i;
  for (i = 0; i < calendar->numberOfDayProfiles; i++) {
    calendar->normalDays[i].id = i + 1; // day ids start at one
    calendar->normalDays[i].numberOfScheduleEntries = 5;
    setupTestScheduleEntries(&(calendar->normalDays[i]), i);
  }
  for (i = 0; i < calendar->numberOfSpecialDayProfiles; i++) {
    calendar->specialDays[i].startDate.year       = i + offset + 114;
    calendar->specialDays[i].startDate.month      = i + offset + 1;
    calendar->specialDays[i].startDate.dayOfMonth = i + offset + 1;
    calendar->specialDays[i].startDate.dayOfWeek  = 0x01;
    calendar->specialDays[i].normalDayIndex = 6;
    calendar->specialDays[i].flags = 0;
  }
}

static void setupTestWeeks(sl_zigbee_af_calendar_struct_t* calendar, uint8_t offset)
{
  uint8_t i;
  for (i = 0; i < calendar->numberOfWeekProfiles; i++) {
    uint8_t j;
    calendar->weeks[i].id = i + 1; // week ids start at one
    for (j = 0; j < SL_ZIGBEE_AF_DAYS_IN_THE_WEEK; j++) {
      calendar->weeks[i].normalDayIndexes[j] = (j < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX
                                                ? j
                                                : SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID);
    }
  }
}

static void setupTestSeasons(sl_zigbee_af_calendar_struct_t* calendar, uint8_t offset)
{
  uint8_t i;
  for (i = 0; i < calendar->numberOfSeasons; i++) {
    if (calendar->weeks[i].id != SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID) {
      calendar->seasons[i].startDate.year       = i + offset + 114;
      calendar->seasons[i].startDate.month      = i + offset + 1;
      calendar->seasons[i].startDate.dayOfMonth = i + offset + 1;
      calendar->seasons[i].startDate.dayOfWeek  = 0x01;
      calendar->seasons[i].weekIndex = i;
    } else {
      calendar->seasons[i].weekIndex = SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID;
    }
  }
}

void sl_zigbee_af_calendar_common_print_summary_command(sl_cli_command_arg_t *arguments)
{
  if (!printSelectedCalendar()) {
    return;
  }

  printCalendar(&(calendars[calendarIndexForPrinting]));
  sl_zigbee_af_calendar_cluster_println("");
}

void sl_zigbee_af_calendar_common_select_command(sl_cli_command_arg_t *arguments)
{
  uint8_t temp = sl_cli_get_argument_uint8(arguments, 0);
  if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= temp) {
    sl_zigbee_af_calendar_cluster_println("Error: Invalid index.  Max is %d.",
                                          SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return;
  }
  calendarIndexForPrinting = temp;
}

void sl_zigbee_af_calendar_common_print_weeks_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i;

  if (!printSelectedCalendar()) {
    return;
  }

  sl_zigbee_af_calendar_cluster_println("Week Profiles");
  for (i = 0; i <  calendars[calendarIndexForPrinting].numberOfWeekProfiles; i++) {
    sl_zigbee_af_calendar_week_struct_t *week = &(calendars[calendarIndexForPrinting].weeks[i]);
    sl_zigbee_af_calendar_cluster_println("  Week ID: %d", week->id);
    sl_zigbee_af_calendar_cluster_println("    Monday    (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[0]].id);
    sl_zigbee_af_calendar_cluster_println("    Tuesday   (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[1]].id);
    sl_zigbee_af_calendar_cluster_println("    Wednesday (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[2]].id);
    sl_zigbee_af_calendar_cluster_println("    Thursday  (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[3]].id);
    sl_zigbee_af_calendar_cluster_println("    Friday    (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[4]].id);
    sl_zigbee_af_calendar_cluster_println("    Saturday  (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[5]].id);
    sl_zigbee_af_calendar_cluster_println("    Sunday    (Day ID Ref):      0x%x", calendars[calendarIndexForPrinting].normalDays[week->normalDayIndexes[6]].id);
  }
}

void sl_zigbee_af_calendar_common_print_days_command(sl_cli_command_arg_t *arguments)
{
  if (!printSelectedCalendar()) {
    return;
  }

  if (memcmp(arguments->argv[arguments->arg_ofs - 1], "days", strlen("days")) == 0) {
    sl_zigbee_af_calendar_cluster_println("Normal Day Profiles");
    printDayProfiles(&(calendars[calendarIndexForPrinting]), false);  // special days?
  } else {
    sl_zigbee_af_calendar_cluster_println("Special Day Profiles");
    printDayProfiles(&(calendars[calendarIndexForPrinting]), true);   // special days?
  }
}

void sl_zigbee_af_calendar_common_setup_test_calendars_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX >= 7       \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX >= 5 \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX >= 4     \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX >= 4   \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX >= 5
  uint8_t i;
  for (i = 0; i < 2; i++) {
    if (SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS <= i) {
      sl_zigbee_af_calendar_cluster_println("Calendar at index %d could not be accessed in setup.", i);
      continue;
    }
    sl_zigbee_af_copy_string(calendars[i].name,
                             (i == 0 ? calendarName0 : calendarName1),
                             SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH);

    calendars[i].calendarId = CALENDAR_ID_START + i;
    calendars[i].startTimeUtc = (i * 1000);
    calendars[i].providerId = PROVIDER_ID_START + i;
    calendars[i].issuerEventId = ISSUER_EVENT_ID_START + i;
    calendars[i].calendarType = SL_ZIGBEE_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR;
    calendars[i].numberOfSeasons = 4;
    calendars[i].numberOfDayProfiles = 7;
    calendars[i].numberOfWeekProfiles = 4;
    calendars[i].numberOfSpecialDayProfiles = 5;
    calendars[i].flags = 0;

    setupTestDays(&(calendars[i]), i);
    setupTestWeeks(&(calendars[i]), i);
    setupTestSeasons(&(calendars[i]), i);
  }
  sl_zigbee_af_calendar_cluster_println("Calendars initialized with test-data.");
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

void sl_zigbee_af_calendar_common_print_seasons_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i;

  if (!printSelectedCalendar()) {
    return;
  }

  sl_zigbee_af_calendar_cluster_println("Seasons");
  for (i = 0; i <  calendars[calendarIndexForPrinting].numberOfSeasons; i++) {
    sl_zigbee_af_calendar_cluster_println("Season: %d", i);
    if (calendars[calendarIndexForPrinting].seasons[i].weekIndex == SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID) {
      sl_zigbee_af_calendar_cluster_println("  No data");
    } else {
      sl_zigbee_af_calendar_cluster_print("  Start Date:  ");
      sl_zigbee_af_print_dateln(&calendars[calendarIndexForPrinting].seasons[i].startDate);
      sl_zigbee_af_calendar_cluster_println("  Week ID: %d", calendars[calendarIndexForPrinting].weeks[calendars[calendarIndexForPrinting].seasons[i].weekIndex].id);
    }
  }
}

void sl_zigbee_af_calendar_common_clear_calendars_command(sl_cli_command_arg_t *arguments)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    memset(&(calendars[i]), 0, sizeof(sl_zigbee_af_calendar_struct_t));
    calendars[i].calendarId = SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID;
  }
}

// Load the Simple Calendar as defined in Appendix D.1 of the SE 1.2 test spec.
void sl_zigbee_af_calendar_common_load_simple_calendar_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX >= 3       \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX >= 2 \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX >= 2     \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX >= 4
  uint8_t length;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (index < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    calendars[index].providerId = sl_cli_get_argument_uint32(arguments, 1);
    calendars[index].issuerEventId = sl_cli_get_argument_uint32(arguments, 2);
    calendars[index].calendarId = sl_cli_get_argument_uint32(arguments, 3);
    calendars[index].startTimeUtc = sl_cli_get_argument_uint32(arguments, 4);
    if (calendars[index].startTimeUtc == 0) {
      calendars[index].startTimeUtc = sl_zigbee_af_get_current_time();
    }
    calendars[index].calendarType = sl_cli_get_argument_uint8(arguments, 5);

    length = sl_zigbee_copy_string_arg(arguments,
                                       6,
                                       calendars[index].name + 1,
                                       SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH, false);
    calendars[index].name[0] = length;
    calendars[index].flags = 0;

    // Day Profiles
    calendars[index].numberOfDayProfiles = 3;
    // Day Profile 1
    calendars[index].normalDays[0].id = 1;
    calendars[index].normalDays[0].numberOfScheduleEntries = 2;
    calendars[index].normalDays[0].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[0].scheduleEntries[0].data = 0x01;
    calendars[index].normalDays[0].scheduleEntries[1].minutesFromMidnight = 0x01A4;
    calendars[index].normalDays[0].scheduleEntries[1].data = 0x02;
    // Day Profile 2
    calendars[index].normalDays[1].id = 2;
    calendars[index].normalDays[1].numberOfScheduleEntries = 2;
    calendars[index].normalDays[1].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[1].scheduleEntries[0].data = 0x01;
    calendars[index].normalDays[1].scheduleEntries[1].minutesFromMidnight = 0x01E0;
    calendars[index].normalDays[1].scheduleEntries[1].data = 0x02;
    // Day Profile 3
    calendars[index].normalDays[2].id = 3;
    calendars[index].normalDays[2].numberOfScheduleEntries = 1;
    calendars[index].normalDays[2].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[2].scheduleEntries[0].data = 0x01;

    // Week Profiles
    calendars[index].numberOfWeekProfiles = 2;
    // Week Profile 1
    calendars[index].weeks[0].id = 1;
    calendars[index].weeks[0].normalDayIndexes[0] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[1] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[2] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[3] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[4] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[5] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[6] = 0x00;
    // Week Profile 2
    calendars[index].weeks[1].id = 2;
    calendars[index].weeks[1].normalDayIndexes[0] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[1] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[2] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[3] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[4] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[5] = 0x01;
    calendars[index].weeks[1].normalDayIndexes[6] = 0x01;

    // Seasons Table
    calendars[index].numberOfSeasons = 4;
    sl_zigbee_af_decode_date(0x71010102, &calendars[index].seasons[0].startDate);
    calendars[index].seasons[0].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x71031F07, &calendars[index].seasons[1].startDate);
    calendars[index].seasons[1].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x710A1F04, &calendars[index].seasons[2].startDate);
    calendars[index].seasons[2].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x72031F01, &calendars[index].seasons[3].startDate);
    calendars[index].seasons[3].weekIndex = 0x01;

    // Special Days Table
    calendars[index].numberOfSpecialDayProfiles = 0;
  } else {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
  }
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

// Load the Enhanced Calendar as defined in Appendix D.2 of the SE 1.2 test spec.
void sl_zigbee_af_calendar_common_load_enhanced_calendar_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX >= 6        \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX >= 48 \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX >= 2      \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX >= 11
  uint8_t length;
  uint8_t i;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (index < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    calendars[index].providerId = sl_cli_get_argument_uint32(arguments, 1);
    calendars[index].issuerEventId = sl_cli_get_argument_uint32(arguments, 2);
    calendars[index].calendarId = sl_cli_get_argument_uint32(arguments, 3);
    calendars[index].startTimeUtc = sl_cli_get_argument_uint32(arguments, 4);
    if (calendars[index].startTimeUtc == 0) {
      calendars[index].startTimeUtc = sl_zigbee_af_get_current_time();
    }
    calendars[index].calendarType = sl_cli_get_argument_uint8(arguments, 5);
    length = sl_zigbee_copy_string_arg(arguments,
                                       6,
                                       calendars[index].name + 1,
                                       SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH, false);
    calendars[index].name[0] = length;
    calendars[index].flags = 0;

    // Day Profiles
    calendars[index].numberOfDayProfiles = 6;
    // Day Profile 1
    calendars[index].normalDays[0].id = 1;
    calendars[index].normalDays[0].numberOfScheduleEntries = 48;
    for (i = 0; i < calendars[index].normalDays[0].numberOfScheduleEntries; i++) {
      calendars[index].normalDays[0].scheduleEntries[i].minutesFromMidnight = i * 0x001E;
      calendars[index].normalDays[0].scheduleEntries[i].data = ((i & 1) == 0) ? 0x01 : 0x02;
    }
    // Day Profile 2
    calendars[index].normalDays[1].id = 2;
    calendars[index].normalDays[1].numberOfScheduleEntries = 5;
    calendars[index].normalDays[1].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[1].scheduleEntries[0].data = 0x01;
    calendars[index].normalDays[1].scheduleEntries[1].minutesFromMidnight = 0x01A4;
    calendars[index].normalDays[1].scheduleEntries[1].data = 0x02;
    calendars[index].normalDays[1].scheduleEntries[2].minutesFromMidnight = 0x021C;
    calendars[index].normalDays[1].scheduleEntries[2].data = 0x01;
    calendars[index].normalDays[1].scheduleEntries[3].minutesFromMidnight = 0x03C0;
    calendars[index].normalDays[1].scheduleEntries[3].data = 0x02;
    calendars[index].normalDays[1].scheduleEntries[4].minutesFromMidnight = 0x0474;
    calendars[index].normalDays[1].scheduleEntries[4].data = 0x01;
    // Day Profile 3
    calendars[index].normalDays[2].id = 3;
    calendars[index].normalDays[2].numberOfScheduleEntries = 1;
    calendars[index].normalDays[2].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[2].scheduleEntries[0].data = 0x01;
    // Day Profile 4
    calendars[index].normalDays[3].id = 4;
    calendars[index].normalDays[3].numberOfScheduleEntries = 48;
    for (i = 0; i < calendars[index].normalDays[3].numberOfScheduleEntries; i++) {
      calendars[index].normalDays[3].scheduleEntries[i].minutesFromMidnight = i * 0x001E;
      calendars[index].normalDays[3].scheduleEntries[i].data = ((i & 1) == 0) ? 0x02 : 0x01;
    }
    // Day Profile 5
    calendars[index].normalDays[4].id = 5;
    calendars[index].normalDays[4].numberOfScheduleEntries = 5;
    calendars[index].normalDays[4].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[4].scheduleEntries[0].data = 0x01;
    calendars[index].normalDays[4].scheduleEntries[1].minutesFromMidnight = 0x01E0;
    calendars[index].normalDays[4].scheduleEntries[1].data = 0x02;
    calendars[index].normalDays[4].scheduleEntries[2].minutesFromMidnight = 0x0258;
    calendars[index].normalDays[4].scheduleEntries[2].data = 0x01;
    calendars[index].normalDays[4].scheduleEntries[3].minutesFromMidnight = 0x03FC;
    calendars[index].normalDays[4].scheduleEntries[3].data = 0x02;
    calendars[index].normalDays[4].scheduleEntries[4].minutesFromMidnight = 0x04B0;
    calendars[index].normalDays[4].scheduleEntries[4].data = 0x01;
    // Day Profile 6
    calendars[index].normalDays[5].id = 6;
    calendars[index].normalDays[5].numberOfScheduleEntries = 1;
    calendars[index].normalDays[5].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[5].scheduleEntries[0].data = 0x01;

    // Week Profiles
    calendars[index].numberOfWeekProfiles = 2;
    // Week Profile 1
    calendars[index].weeks[0].id = 1;
    calendars[index].weeks[0].normalDayIndexes[0] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[1] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[2] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[3] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[4] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[5] = 0x01;
    calendars[index].weeks[0].normalDayIndexes[6] = 0x02;
    // Week Profile 2
    calendars[index].weeks[1].id = 2;
    calendars[index].weeks[1].normalDayIndexes[0] = 0x03;
    calendars[index].weeks[1].normalDayIndexes[1] = 0x03;
    calendars[index].weeks[1].normalDayIndexes[2] = 0x03;
    calendars[index].weeks[1].normalDayIndexes[3] = 0x03;
    calendars[index].weeks[1].normalDayIndexes[4] = 0x03;
    calendars[index].weeks[1].normalDayIndexes[5] = 0x04;
    calendars[index].weeks[1].normalDayIndexes[6] = 0x05;

    // Seasons Table

    calendars[index].numberOfSeasons = 11;
    sl_zigbee_af_decode_date(0x71010102, &calendars[index].seasons[0].startDate);
    calendars[index].seasons[0].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x71031F07, &calendars[index].seasons[1].startDate);
    calendars[index].seasons[1].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x710A1F04, &calendars[index].seasons[2].startDate);
    calendars[index].seasons[2].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x72031F01, &calendars[index].seasons[3].startDate);
    calendars[index].seasons[3].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x720A1F05, &calendars[index].seasons[4].startDate);
    calendars[index].seasons[4].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x73031F02, &calendars[index].seasons[5].startDate);
    calendars[index].seasons[5].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x730A1F06, &calendars[index].seasons[6].startDate);
    calendars[index].seasons[6].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x74031F04, &calendars[index].seasons[7].startDate);
    calendars[index].seasons[7].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x740A1F01, &calendars[index].seasons[8].startDate);
    calendars[index].seasons[8].weekIndex = 0x00;
    sl_zigbee_af_decode_date(0x75031F05, &calendars[index].seasons[9].startDate);
    calendars[index].seasons[9].weekIndex = 0x01;
    sl_zigbee_af_decode_date(0x750A1F02, &calendars[index].seasons[10].startDate);
    calendars[index].seasons[10].weekIndex = 0x00;

    // Special Days Table
    calendars[index].numberOfSpecialDayProfiles = 0;
  } else {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
  }
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

// Load the Flat Calendar as defined in Appendix D.3 of the SE 1.2 test spec.
void sl_zigbee_af_calendar_common_load_flat_calendar_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX >= 1       \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX >= 1 \
  && SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX >= 1
  uint8_t length;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (index < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    calendars[index].providerId = sl_cli_get_argument_uint32(arguments, 1);
    calendars[index].issuerEventId = sl_cli_get_argument_uint32(arguments, 2);
    calendars[index].calendarId = sl_cli_get_argument_uint32(arguments, 3);
    calendars[index].startTimeUtc = sl_cli_get_argument_uint32(arguments, 4);
    if (calendars[index].startTimeUtc == 0) {
      calendars[index].startTimeUtc = sl_zigbee_af_get_current_time();
    }
    calendars[index].calendarType = sl_cli_get_argument_uint8(arguments, 5);
    length = sl_zigbee_copy_string_arg(arguments,
                                       6,
                                       calendars[index].name + 1,
                                       SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH, false);
    calendars[index].name[0] = length;
    calendars[index].flags = 0;

    // Day Profiles
    calendars[index].numberOfDayProfiles = 1;
    // Day Profile 1
    calendars[index].normalDays[0].id = 1;
    calendars[index].normalDays[0].numberOfScheduleEntries = 1;
    calendars[index].normalDays[0].scheduleEntries[0].minutesFromMidnight = 0x0000;
    calendars[index].normalDays[0].scheduleEntries[0].data = 0x01;

    // Week Profiles
    calendars[index].numberOfWeekProfiles = 1;
    // Week Profile 1
    calendars[index].weeks[0].id = 1;
    calendars[index].weeks[0].normalDayIndexes[0] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[1] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[2] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[3] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[4] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[5] = 0x00;
    calendars[index].weeks[0].normalDayIndexes[6] = 0x00;

    // Seasons Table
    calendars[index].numberOfSeasons = 0;

    // Special Days Table
    calendars[index].numberOfSpecialDayProfiles = 0;
  } else {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
  }
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

void sl_zigbee_af_calendar_common_load_simple_special_days_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX >= 7
  uint8_t i;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (index < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    calendars[index].numberOfSpecialDayProfiles = 7;
    for (i = 0; i < calendars[index].numberOfSpecialDayProfiles; i++) {
      calendars[index].specialDays[i].normalDayIndex = 0x03;
      calendars[index].specialDays[i].flags = 0;
    }
    sl_zigbee_af_decode_date(0x71010102, &calendars[index].specialDays[0].startDate); // (1st January 2013)
    sl_zigbee_af_decode_date(0x71031D05, &calendars[index].specialDays[1].startDate); // (29th March 2013)
    sl_zigbee_af_decode_date(0x71040101, &calendars[index].specialDays[2].startDate); // (1st April 2013)
    sl_zigbee_af_decode_date(0x71051B01, &calendars[index].specialDays[3].startDate); // (27th May 2013)
    sl_zigbee_af_decode_date(0x71081A01, &calendars[index].specialDays[4].startDate); // (26th August 2013)
    sl_zigbee_af_decode_date(0x710C1903, &calendars[index].specialDays[5].startDate); // (25th December 2013)
    sl_zigbee_af_decode_date(0x710C1A04, &calendars[index].specialDays[6].startDate); // (26th December 2013)
  } else {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
  }
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

void sl_zigbee_af_calendar_common_load_enhanced_special_days_command(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX >= 50
  uint8_t i;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  if (index < SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS) {
    calendars[index].numberOfSpecialDayProfiles = 50;
    for (i = 0; i < calendars[index].numberOfSpecialDayProfiles; i++) {
      calendars[index].specialDays[i].normalDayIndex = 0x03;
      calendars[index].specialDays[i].flags = 0;
    }
    sl_zigbee_af_decode_date(0x71010701, &calendars[index].specialDays[0].startDate);  // (7th January 2013)
    sl_zigbee_af_decode_date(0x71010E01, &calendars[index].specialDays[1].startDate);  // (14th January 2013)
    sl_zigbee_af_decode_date(0x71011501, &calendars[index].specialDays[2].startDate);  // (21st January 2013)
    sl_zigbee_af_decode_date(0x71011C01, &calendars[index].specialDays[3].startDate);  // (28th January 2013)
    sl_zigbee_af_decode_date(0x71020401, &calendars[index].specialDays[4].startDate);  // (4th February 2013)
    sl_zigbee_af_decode_date(0x71020B01, &calendars[index].specialDays[5].startDate);  // (11th February 2013)
    sl_zigbee_af_decode_date(0x71021201, &calendars[index].specialDays[6].startDate);  // (18th February 2013)
    sl_zigbee_af_decode_date(0x71021901, &calendars[index].specialDays[7].startDate);  // (25th February 2013)
    sl_zigbee_af_decode_date(0x71030401, &calendars[index].specialDays[8].startDate);  // (4th March 2013)
    sl_zigbee_af_decode_date(0x71030B01, &calendars[index].specialDays[9].startDate);  // (11th March 2013)
    sl_zigbee_af_decode_date(0x71031201, &calendars[index].specialDays[10].startDate); // (18th March 2013)
    sl_zigbee_af_decode_date(0x71031901, &calendars[index].specialDays[11].startDate); // (25th March 2013)
    sl_zigbee_af_decode_date(0x71040101, &calendars[index].specialDays[12].startDate); // (1st April 2013)
    sl_zigbee_af_decode_date(0x71040801, &calendars[index].specialDays[13].startDate); // (8th April 2013)
    sl_zigbee_af_decode_date(0x71040F01, &calendars[index].specialDays[14].startDate); // (15th April 2013)
    sl_zigbee_af_decode_date(0x71041601, &calendars[index].specialDays[15].startDate); // (22nd April 2013)
    sl_zigbee_af_decode_date(0x71041D01, &calendars[index].specialDays[16].startDate); // (29th April 2013)
    sl_zigbee_af_decode_date(0x71050601, &calendars[index].specialDays[17].startDate); // (6th May 2013)
    sl_zigbee_af_decode_date(0x71050D01, &calendars[index].specialDays[18].startDate); // (13th May 2013)
    sl_zigbee_af_decode_date(0x71051401, &calendars[index].specialDays[19].startDate); // (20th May 2013)
    sl_zigbee_af_decode_date(0x71051B01, &calendars[index].specialDays[20].startDate); // (27th May 2013)
    sl_zigbee_af_decode_date(0x71060301, &calendars[index].specialDays[21].startDate); // (3rd June 2013)
    sl_zigbee_af_decode_date(0x71060A01, &calendars[index].specialDays[22].startDate); // (10th June 2013)
    sl_zigbee_af_decode_date(0x71061101, &calendars[index].specialDays[23].startDate); // (17th June 2013)
    sl_zigbee_af_decode_date(0x71061801, &calendars[index].specialDays[24].startDate); // (24th June 2013)
    sl_zigbee_af_decode_date(0x71070101, &calendars[index].specialDays[25].startDate); // (1st July 2013)
    sl_zigbee_af_decode_date(0x71070801, &calendars[index].specialDays[26].startDate); // (8th July 2013)
    sl_zigbee_af_decode_date(0x71070F01, &calendars[index].specialDays[27].startDate); // (15th July 2013)
    sl_zigbee_af_decode_date(0x71071601, &calendars[index].specialDays[28].startDate); // (22nd July 2013)
    sl_zigbee_af_decode_date(0x71071D01, &calendars[index].specialDays[29].startDate); // (29th July 2013)
    sl_zigbee_af_decode_date(0x71080501, &calendars[index].specialDays[30].startDate); // (5th August 2013)
    sl_zigbee_af_decode_date(0x71080C01, &calendars[index].specialDays[31].startDate); // (12th August 2013)
    sl_zigbee_af_decode_date(0x71081301, &calendars[index].specialDays[32].startDate); // (19th August 2013)
    sl_zigbee_af_decode_date(0x71081A01, &calendars[index].specialDays[33].startDate); // (26th August 2013)
    sl_zigbee_af_decode_date(0x71090201, &calendars[index].specialDays[34].startDate); // (2nd September 2013)
    sl_zigbee_af_decode_date(0x71090901, &calendars[index].specialDays[35].startDate); // (9th September 2013)
    sl_zigbee_af_decode_date(0x71091001, &calendars[index].specialDays[36].startDate); // (16th September 2013)
    sl_zigbee_af_decode_date(0x71091701, &calendars[index].specialDays[37].startDate); // (23rd September 2013)
    sl_zigbee_af_decode_date(0x71091E01, &calendars[index].specialDays[38].startDate); // (30th September 2013)
    sl_zigbee_af_decode_date(0x710A0701, &calendars[index].specialDays[39].startDate); // (7th October 2013)
    sl_zigbee_af_decode_date(0x710A0E01, &calendars[index].specialDays[40].startDate); // (14th October 2013)
    sl_zigbee_af_decode_date(0x710A1501, &calendars[index].specialDays[41].startDate); // (21st October 2013)
    sl_zigbee_af_decode_date(0x710A1C01, &calendars[index].specialDays[42].startDate); // (28th October 2013)
    sl_zigbee_af_decode_date(0x710B0401, &calendars[index].specialDays[43].startDate); // (4th November 2013)
    sl_zigbee_af_decode_date(0x710B0B01, &calendars[index].specialDays[44].startDate); // (11th November 2013)
    sl_zigbee_af_decode_date(0x710B1201, &calendars[index].specialDays[45].startDate); // (18th November 2013)
    sl_zigbee_af_decode_date(0x710B1901, &calendars[index].specialDays[46].startDate); // (25th November 2013)
    sl_zigbee_af_decode_date(0x710C0201, &calendars[index].specialDays[47].startDate); // (2nd December 2013)
    sl_zigbee_af_decode_date(0x710C0901, &calendars[index].specialDays[48].startDate); // (9th December 2013)
    sl_zigbee_af_decode_date(0x710C1001, &calendars[index].specialDays[49].startDate); // (16th December 2013)
  } else {
    sl_zigbee_af_calendar_cluster_println("Index must be in the range of 0 to %d", SL_ZIGBEE_AF_PLUGIN_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
  }
#else
  sl_zigbee_af_calendar_cluster_println("This command is not available with current set of configuration");
#endif
}

// plugin calendar-common set-cal-info <index:1> <providerId:1> <issuerEventId:1>
//                                     <calId:1> <startTime:4> <calType:1>
//                                     <calName:string> <numberOfSeasons:1>
//                                     <numberOfWeekProfiles:1>
//                                     <numberOfDayProfiles:1>
void sl_zigbee_af_calendar_common_cli_set_cal_info(sl_cli_command_arg_t *arguments)
{
  uint8_t calName[SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1];
  uint8_t length;

  length = sl_zigbee_copy_string_arg(arguments,
                                     6,
                                     calName + 1,
                                     SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH, false);

  calName[0] = length;
  sl_zigbee_af_calendar_common_set_cal_info(sl_cli_get_argument_uint8(arguments, 0),
                                            sl_cli_get_argument_uint32(arguments, 1),
                                            sl_cli_get_argument_uint32(arguments, 2),
                                            sl_cli_get_argument_uint32(arguments, 3),
                                            sl_cli_get_argument_uint32(arguments, 4),
                                            sl_cli_get_argument_uint8(arguments, 5),
                                            calName,
                                            sl_cli_get_argument_uint8(arguments, 7),
                                            sl_cli_get_argument_uint8(arguments, 8),
                                            sl_cli_get_argument_uint8(arguments, 9));
}

// plugin calendar-common add-cal-info <providerId:1> <issuerEventId:1>
//                                     <calId:1> <startTime:4> <calType:1>
//                                     <calName:string> <numberOfSeasons:1>
//                                     <numberOfWeekProfiles:1>
//                                     <numberOfDayProfiles:1>
void sl_zigbee_af_calendar_common_cli_add_cal_info(sl_cli_command_arg_t *arguments)
{
  uint8_t calName[SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1];
  uint8_t length;
  length = sl_zigbee_copy_string_arg(arguments,
                                     5,
                                     calName + 1,
                                     SL_ZIGBEE_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH, false);
  calName[0] = length;
  sl_zigbee_af_calendar_common_add_cal_info(sl_cli_get_argument_uint32(arguments, 0),
                                            sl_cli_get_argument_uint32(arguments, 1),
                                            sl_cli_get_argument_uint32(arguments, 2),
                                            sl_cli_get_argument_uint32(arguments, 3),
                                            sl_cli_get_argument_uint8(arguments, 4),
                                            calName,
                                            sl_cli_get_argument_uint8(arguments, 6),
                                            sl_cli_get_argument_uint8(arguments, 7),
                                            sl_cli_get_argument_uint8(arguments, 8));
}

// plugin calendar-common set-day-prof-info <index:1> <dayId:1> <entryId:1>
//                                          <minutesFromMidnight:2> <data:1>
void sl_zigbee_af_calendar_common_cli_set_day_prof_info(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_calendar_common_set_day_prof_info(sl_cli_get_argument_uint8(arguments, 0),
                                                 sl_cli_get_argument_uint8(arguments, 1),
                                                 sl_cli_get_argument_uint8(arguments, 2),
                                                 sl_cli_get_argument_uint16(arguments, 3),
                                                 sl_cli_get_argument_uint8(arguments, 4));
}

// plugin calendar-common set-week-prof-info <index:1> <weekId:1> <dayIdRefMon:1>
//                                           <dayIdRefTue:1> <dayIdRefWed:1>
//                                           <dayIdRefThu:1> <dayInRefFri:1>
//                                           <dayIdRefSat:1> <dayIdRefSun:1>
void sl_zigbee_af_calendar_common_cli_set_week_prof_info(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_calendar_server_set_week_prof_info(sl_cli_get_argument_uint8(arguments, 0),
                                                  sl_cli_get_argument_uint8(arguments, 1),
                                                  sl_cli_get_argument_uint8(arguments, 2),
                                                  sl_cli_get_argument_uint8(arguments, 3),
                                                  sl_cli_get_argument_uint8(arguments, 4),
                                                  sl_cli_get_argument_uint8(arguments, 5),
                                                  sl_cli_get_argument_uint8(arguments, 6),
                                                  sl_cli_get_argument_uint8(arguments, 7),
                                                  sl_cli_get_argument_uint8(arguments, 8));
}

// plugin calendar-common set-seasons-info <index:1> <seasonId:1> <startDate:4> <weekIndex:1>
void sl_zigbee_af_calendar_common_cli_set_seasons_info(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_date_t date;
  uint32_t tmp = sl_cli_get_argument_uint32(arguments, 2);
  memset(&date, tmp, sizeof(sl_zigbee_af_date_t));
  sl_zigbee_af_calendar_server_set_seasons_info(sl_cli_get_argument_uint8(arguments, 0),
                                                sl_cli_get_argument_uint8(arguments, 1),
                                                date,
                                                sl_cli_get_argument_uint8(arguments, 3));
}
