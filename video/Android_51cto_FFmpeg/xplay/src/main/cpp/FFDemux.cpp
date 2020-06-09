//
// Created by kevint on 6/9/20.
//

#include "FFDemux.h"
#include "XLog.h"

bool FFDemux::open(const char *url) {

    XLOGI("open file %s", url);
    return true;
}

XData FFDemux::read() {
    XData d;

    return d;
}
