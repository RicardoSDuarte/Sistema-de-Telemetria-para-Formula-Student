#include "CANFilter.h"
#include <algorithm>

std::vector<uint32_t> CANFilter::validIds = {
    0x130, 0x131, 0x132, 0x133, 0x134,
    0x135, 0x136, 0x137, 0x138, 0x139,
    0x13A, 0x13B, 0x13C, 0x13D, 0x13E
};

bool CANFilter::filterEnabled = true;

bool CANFilter::isValidId(const CAN_FRAME& msg) {
    if (!filterEnabled) return true;

    return std::find(validIds.begin(), validIds.end(), msg.id) != validIds.end();
}

void CANFilter::insertID(uint32_t id) {
    if (std::find(validIds.begin(), validIds.end(), id) == validIds.end()) {
        validIds.push_back(id);
    }
}

void CANFilter::deleteID(uint32_t id) {
    auto it = std::find(validIds.begin(), validIds.end(), id);
    if (it != validIds.end()) {
        validIds.erase(it);
    }
}

void CANFilter::sortIDs() {
    std::sort(validIds.begin(), validIds.end());
}

void CANFilter::clearList() {
    validIds.clear();
}

void CANFilter::setFilterEnabled(bool enabled) {
    filterEnabled = enabled;
}

bool CANFilter::isFilterEnabled() {
    return filterEnabled;
}
