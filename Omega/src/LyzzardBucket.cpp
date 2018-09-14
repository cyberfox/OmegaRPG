#include "defs.h"
#include "LyzzardBucket.h"

LyzzardBucket::LyzzardBucket() : Food() {
    initialize();
}

void LyzzardBucket::initialize() {
    id = FOODID + 0;
    objstr = "red and white striped bucket";
    truename = "Lyzzard Bucket";
    cursestr = "Lyzzard Bucket";
}

LyzzardBucket::~LyzzardBucket(void) = default;

LyzzardBucket::LyzzardBucket(Object *o) : Food(o) {
    initialize();
}
