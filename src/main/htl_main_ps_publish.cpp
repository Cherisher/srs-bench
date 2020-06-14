/*
The MIT License (MIT)

Copyright (c) 2013-2015 winlin

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <htl_stdinc.hpp>

#include <getopt.h>
#include <stdlib.h>

#include <string>
using namespace std;

// project lib
#include <htl_core_log.hpp>
#include <htl_core_error.hpp>
#include <htl_app_rtmp_load.hpp>
#include <htl_app_rtmp_protocol.hpp>
#include <htl_app_rtmp_publish.hpp>

#include <htl_main_utility.hpp>

#define DefaultDelaySeconds 1.0
#define DefaultRtmpUrlSingle "rtp://127.0.0.1:9000"
#define DefaultRtmpUrl "rtp://127.0.0.1:9000"
#define DefaultInputFlv "doc/test.mpg"

int discovery_options(int argc, char** argv, 
    bool& show_help, bool& show_version, string& url, int& threads, 
    double& startup, double& delay, double& error, double& report, int& count,
    string& input, int &port, string &addr
){
    int ret = ERROR_SUCCESS;
    
    static option long_options[] = {
        SharedOptions()
        {"input", no_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"addr", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };
    
    int opt = 0;
    int option_index = 0;
    while((opt = getopt_long(argc, argv, "hvc:r:t:s:d:e:m:i:p:a:", long_options, &option_index)) != -1){
        switch(opt){
            ProcessSharedOptions()
            case 'i':
                input = optarg;
                break;
            default:
                show_help = true;
                break;
            case 'p': 
                port = atof(optarg); 
                break;
            case 'a': 
                addr = atof(optarg); 
                break;
        }
    }
    
    // check values
    if(url == "" || input == ""){
        show_help = true;
        return ret;
    }

    return ret;
}

void help(char** argv){
    printf("%s, Copyright (c) 2013-2015 winlin\n", ProductRtmpPublishName);
    printf("srs.librtmp %d.%d.%d (https://github.com/winlinvip/srs.librtmp)\n\n", 
        srs_version_major(), srs_version_minor(), srs_version_revision());
    
    printf(""
        "Usage: %s <Options> <-u URL>\n"
        "%s base on st(state-threads), support huge concurrency.\n"
        "Options:\n"
        ShowHelpPart1()
        ShowHelpPart2()
        "\n"
        "Examples:\n"
        "1. start a client\n"
        "   %s -i %s -c 1 -r %s\n"
        "2. start 1000 clients\n"
        "   %s -i %s -c 1000 -r %s\n"
        "3. start 10000 clients\n"
        "   %s -i %s -c 10000 -r %s\n"
        "4. start 100000 clients\n"
        "   %s -i %s -c 100000 -r %s\n"
        "\n"
        "This program built for %s.\n"
        "Report bugs to <%s>\n",
        argv[0], argv[0], 
        DefaultThread, DefaultRtmpUrl, DefaultCount, // part1
        (double)DefaultStartupSeconds, DefaultDelaySeconds, // part2
        DefaultErrorSeconds, DefaultReportSeconds, // part2
        argv[0], DefaultInputFlv, DefaultRtmpUrlSingle, 
        argv[0], DefaultInputFlv, DefaultRtmpUrl, 
        argv[0], DefaultInputFlv, DefaultRtmpUrl, 
        argv[0], DefaultInputFlv, DefaultRtmpUrl,
        BuildPlatform, BugReportEmail);
        
    exit(0);
}

int main(int argc, char** argv){
    int ret = ERROR_SUCCESS;
    
    bool show_help = false, show_version = false; 
    string url; int threads = DefaultThread; 
    double start = DefaultStartupSeconds, delay = DefaultDelaySeconds, error = DefaultErrorSeconds;
    double report = DefaultReportSeconds; int count = DefaultCount;
    string input, addr;
    int port=0;
    
    if((ret = discovery_options(argc, argv, show_help, show_version, url, threads, start, delay, error, report, count, input, port, addr)) != ERROR_SUCCESS){
        Error("discovery options failed. ret=%d", ret);
        return ret;
    }
    
    if(show_help){
        help(argv);
    }
    if(show_version){
        version();
    }
    
    Trace("params url=%s, threads=%d, start=%.2f, delay=%.2f, error=%.2f, report=%.2f, count=%d", 
        url.c_str(), threads, start, delay, error, report, count);

    //SrsPsStreamClient ps("a", false, false);
    //ps.init_sock("127.0.0.1", 9000);
    char *psdata=NULL;
    long size = 0;
    SrsPsStreamClient::read_ps_file(input, &psdata, &size);
    // long size = 0;
    // SrsPsStreamClient::read_ps_file("test.mpg", &psdata, &size);
    // Trace("file=%d, size=%u", psdata, size);
    // if(!psdata || !size){
    //     Error("psdata is empty");
    //     return ret;
    // }
    
    StFarm farm;
    
    if((ret = farm.Initialize(report)) != ERROR_SUCCESS){
        Error("initialize the farm failed. ret=%d", ret);
        return ret;
    }

    vector<string> vecUrls;
    SplitString(url, vecUrls,","); //可按多个字符来分隔;
    //count = vecUrls.size() + 1
    for(vector<string>::size_type j = 0; j != vecUrls.size(); ++j){
        for(int i = 0; i < threads; i++){
            StPsPublishTask* task = new StPsPublishTask();

            task->copy_psdata(psdata, size);
            task->ssrc = i + 1;
            // Trace("===%x, %x, %x, %x", task->psdata[0], task->psdata[1], 
            //     task->psdata[2], task->psdata[3]);

            char index[16];
            snprintf(index, sizeof(index), "%d", i);
            
            
            std::string _index = index;
            std::string rtmp_url =  vecUrls[j];
            size_t pos = std::string::npos;
            if ((pos = rtmp_url.find("{i}")) != std::string::npos) {
                rtmp_url = rtmp_url.replace(pos, 3, _index);
            }
            
            if((ret = task->Initialize(input, rtmp_url, start, delay, error, count, port)) != ERROR_SUCCESS){
                Error("initialize task failed, input=%s, url=%s, ret=%d", input.c_str(), rtmp_url.c_str(), ret);
                srs_freep(psdata);
                return ret;
            }
            
            if((ret = farm.Spawn(task)) != ERROR_SUCCESS){
                Error("st farm spwan task failed, ret=%d", ret);
                srs_freep(psdata);
                return ret;
            }
        }
    }

    srs_freep(psdata);
    
    farm.WaitAll();
    
    return 0;
}
