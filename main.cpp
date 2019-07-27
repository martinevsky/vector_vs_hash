#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>

#include <benchmark/benchmark.h>

#include "report_template.h"

std::string AsString (boost::string_view view)
{
    return {view.begin(), view.end()};
}

class HtmlReporter final : public benchmark::ConsoleReporter
{
    using Base = benchmark::ConsoleReporter;

protected:
    virtual void ReportRuns (const std::vector<Run>& report) override
    {
        m_runs.insert (std::end (m_runs), std::begin (report), std::end (report));
        Base::ReportRuns (report);
    }

public:
    void GenerateReport()
    {
        std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> series;
        for (const auto& run : m_runs)
        {
            if (!run.counters.empty())
            {
                std::vector<std::string> splitRes;
                auto str = run.benchmark_name();
                boost::algorithm::split (splitRes, str, boost::is_any_of("/"));
                if (splitRes.size() != 2)
                    continue;

                series[splitRes[0]].push_back({splitRes[1], run.counters.begin()->second});
            }
        }

        std::stringstream str;
        for (const auto& ser : series)
        {
            str << "{\n";
            str << "\tname: \"" << ser.first << "\",\n";
            str << "\tdata: [";
            
            for (const auto& data : ser.second)
                str << '[' << data.first << ',' << data.second << "],";

            str << "],\n";
            str << "},\n";
        }

        std::string serstr = str.str();
        boost::algorithm::replace_all (serstr, boost::as_literal ("<"), boost::as_literal ("&lt;"));
	    boost::algorithm::replace_all (serstr, boost::as_literal (">"), boost::as_literal ("&gt;"));

        std::cout << serstr;

        std::string report = AsString (GetReportTemplate());
        boost::algorithm::replace_all (report, GetSeriesPlaceholder(), serstr);

        std::ofstream out("report.html");
        out << report;
    }

private:
    std::vector<Run> m_runs;
};

int main (int argc, char **argv)
{
    ::benchmark::Initialize (&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments (argc, argv))
        return 1;

    HtmlReporter htmlReporter;
    ::benchmark::RunSpecifiedBenchmarks (&htmlReporter);
    htmlReporter.GenerateReport();
}
