#pragma once

#include <Arduino.h>

struct BlePipelineStats
{
    uint32_t adv_received = 0;
    uint32_t adv_dropped = 0;
    uint32_t adv_decrypt_fail = 0;

    uint32_t data_enqueued = 0;
    uint32_t data_dropped = 0;
    uint32_t data_processed = 0;

    uint32_t mapped_updates = 0;
    uint32_t direct_updates = 0;
    uint32_t registry_updates = 0;
    uint32_t registry_new = 0;

    uint32_t current_adv_depth = 0;
    uint32_t current_data_depth = 0;
    uint32_t max_adv_depth = 0;
    uint32_t max_data_depth = 0;
    uint32_t max_end_to_end_ms = 0;
};

extern BlePipelineStats bleStats;

void bleStatsReset();
BlePipelineStats bleStatsSnapshot();
void bleStatsRecordAdvReceived(uint32_t depth);
void bleStatsRecordAdvDropped(uint32_t depth);
void bleStatsRecordAdvDecryptFail();
void bleStatsRecordDataEnqueued(uint32_t depth);
void bleStatsRecordDataDropped(uint32_t depth);
void bleStatsRecordProcessed(uint32_t endToEndMs);
void bleStatsRecordMappedUpdate();
void bleStatsRecordDirectUpdate();
void bleStatsRecordRegistryUpdate(bool isNew);
