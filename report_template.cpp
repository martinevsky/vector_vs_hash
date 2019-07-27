#include "report_template.h"

boost::string_view GetReportTemplate()
{
    const static char res[] = 
#include "report_template.html"
    ;
    return res;
}

boost::string_view GetSeriesPlaceholder()
{
    return "[Place series here]";
}