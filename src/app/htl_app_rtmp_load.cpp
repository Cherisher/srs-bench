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

#include <inttypes.h>
#include <stdlib.h>

#include <string>
#include <sstream>
using namespace std;

#include <htl_core_error.hpp>
#include <htl_core_log.hpp>
#include <htl_app_rtmp_play.hpp>
#include <htl_app_rtmp_publish.hpp>
#include <htl_app_ps_publish.hpp>

#include <htl_app_rtmp_load.hpp>

StRtmpTask::StRtmpTask(){
}

StRtmpTask::~StRtmpTask(){
}

int StRtmpTask::Initialize(string http_url, double startup, double delay, double error, int count){
    int ret = ERROR_SUCCESS;
    
    if((ret = InitializeBase(http_url, startup, delay, error, count)) != ERROR_SUCCESS){
        return ret;
    }
    
    return ret;
}

Uri* StRtmpTask::GetUri(){
    return &url;
}

int StRtmpTask::ProcessTask(){
    int ret = ERROR_SUCCESS;
    
    Trace("start to process RTMP play task #%d, schema=%s, host=%s, port=%d, tcUrl=%s, stream=%s, startup=%.2f, delay=%.2f, error=%.2f, count=%d", 
        GetId(), url.GetSchema(), url.GetHost(), url.GetPort(), url.GetTcUrl(), url.GetStream(), startup_seconds, delay_seconds, error_seconds, count);
       
    StRtmpPlayClient client;
    
    // if count is zero, infinity loop.
    for(int i = 0; count == 0 || i < count; i++){
        statistic->OnTaskStart(GetId(), url.GetUrl());
        
        if((ret = client.Dump(&url)) != ERROR_SUCCESS){
            statistic->OnTaskError(GetId(), 0);
            
            Error("rtmp client dump url failed. ret=%d", ret);
            st_usleep((st_utime_t)(error_seconds * 1000 * 1000));
            continue;
        }
        
        int sleep_ms = StUtility::BuildRandomMTime((delay_seconds >= 0)? delay_seconds:0);
        Trace("[RTMP] %s dump success, sleep %dms", url.GetUrl(), sleep_ms);
        st_usleep(sleep_ms * 1000);
        
        statistic->OnTaskEnd(GetId(), 0);
    }
    
    return ret;
}

StRtmpTaskFast::StRtmpTaskFast(){
}

StRtmpTaskFast::~StRtmpTaskFast(){
}

int StRtmpTaskFast::Initialize(string http_url, double startup, double delay, double error, int count){
    int ret = ERROR_SUCCESS;
    
    if((ret = InitializeBase(http_url, startup, delay, error, count)) != ERROR_SUCCESS){
        return ret;
    }
    
    return ret;
}

Uri* StRtmpTaskFast::GetUri(){
    return &url;
}

int StRtmpTaskFast::ProcessTask(){
    int ret = ERROR_SUCCESS;
    
    Trace("start to process RTMP play fast task #%d, schema=%s, host=%s, port=%d, tcUrl=%s, stream=%s, startup=%.2f, delay=%.2f, error=%.2f, count=%d", 
        GetId(), url.GetSchema(), url.GetHost(), url.GetPort(), url.GetTcUrl(), url.GetStream(), startup_seconds, delay_seconds, error_seconds, count);
       
    StRtmpPlayClientFast client;
    
    // if count is zero, infinity loop.
    for(int i = 0; count == 0 || i < count; i++){
        statistic->OnTaskStart(GetId(), url.GetUrl());
        
        if((ret = client.Dump(&url)) != ERROR_SUCCESS){
            statistic->OnTaskError(GetId(), 0);
            
            Error("rtmp client dump url failed. ret=%d", ret);
            st_usleep((st_utime_t)(error_seconds * 1000 * 1000));
            continue;
        }
        
        int sleep_ms = StUtility::BuildRandomMTime((delay_seconds >= 0)? delay_seconds:0);
        Trace("[RTMP] %s dump success, sleep %dms", url.GetUrl(), sleep_ms);
        st_usleep(sleep_ms * 1000);
        
        statistic->OnTaskEnd(GetId(), 0);
    }
    
    return ret;
}

StRtmpPublishTask::StRtmpPublishTask(){
}

StRtmpPublishTask::~StRtmpPublishTask(){
}

int StRtmpPublishTask::Initialize(string input, string http_url, double startup, double delay, double error, int count){
    int ret = ERROR_SUCCESS;
    
    input_flv_file = input;

    if((ret = InitializeBase(http_url, startup, delay, error, count)) != ERROR_SUCCESS){
        return ret;
    }
    
    return ret;
}

Uri* StRtmpPublishTask::GetUri(){
    return &url;
}

int StRtmpPublishTask::ProcessTask(){
    int ret = ERROR_SUCCESS;
    
    Trace("start to process RTMP publish task #%d, schema=%s, host=%s, port=%d, tcUrl=%s, url=%s,stream=%s, startup=%.2f, delay=%.2f, error=%.2f, count=%d", 
        GetId(), url.GetSchema(), url.GetHost(), url.GetPort(), url.GetTcUrl(), url.GetUrl(), url.GetStream(), startup_seconds, delay_seconds, error_seconds, count);
       
    StRtmpPublishClient client;
    
    // if count is zero, infinity loop.
    for(int i = 0; count == 0 || i < count; i++){
        statistic->OnTaskStart(GetId(), url.GetUrl());
        
        if((ret = client.Publish(input_flv_file, &url)) != ERROR_SUCCESS){
            statistic->OnTaskError(GetId(), 0);
            
            Error("rtmp client publish url failed. ret=%d", ret);
            st_usleep((st_utime_t)(error_seconds * 1000 * 1000));
            continue;
        }
        
        int sleep_ms = StUtility::BuildRandomMTime((delay_seconds >= 0)? delay_seconds:0);
        Trace("[RTMP] %s publish success, sleep %dms", url.GetUrl(), sleep_ms);
        st_usleep(sleep_ms * 1000);
        
        statistic->OnTaskEnd(GetId(), 0);
    }
    
    return ret;
}

StPsPublishTask::StPsPublishTask(){
    ssrc = 0;
    StPsPublishTask::count++;
    size = 0;
    start_pos = 0; 
    input_mpg_file = "";
    client = new SrsPsStreamClient("", false, false);
}

StPsPublishTask::~StPsPublishTask(){
    
    if (StPsPublishTask::count>0)
        StPsPublishTask::count--;
    
    if (StPsPublishTask::count==0){
       srs_freepa(StPsPublishTask::psdata);
    }
    
    srs_freep(client);
}

int StPsPublishTask::Initialize(string input, string http_url, double startup, double delay, double error, int count, int start_port){
    int ret = ERROR_SUCCESS;
    
    input_mpg_file = input;
    
    if((ret = InitializeBase(http_url, startup, delay, error, count)) != ERROR_SUCCESS){
        return ret;
    }

    //SrsPsStreamClient  client("", false, false);
    int port = url.GetPort();
    string host = url.GetHost();
    client->init_sock(host, port, start_port);
    start_pos = start_port;
    return ret;
}

Uri* StPsPublishTask::GetUri(){
    return &url;
}

void StPsPublishTask::copy_psdata(char *p, int s){
    if (StPsPublishTask::size != 0)
        return;
    StPsPublishTask::size = s;
    memcpy(StPsPublishTask::psdata, p, s);
}

char* StPsPublishTask::psdata = new char[1024*1024*50];
long StPsPublishTask::size  = 0;
int StPsPublishTask::count = 0;

int StPsPublishTask::ProcessTask(){
    int ret = ERROR_SUCCESS;
    
    Trace("start to process PS publish task #%d, schema=%s, host=%s, port=%d, startup=%.2f, delay=%.2f, error=%.2f, count=%d", 
        GetId(), url.GetSchema(), url.GetHost(), url.GetPort(), startup_seconds, delay_seconds, error_seconds, count);

    char *psdata = StPsPublishTask::psdata;
    long size = StPsPublishTask::size;
    //SrsPsStreamClient::read_ps_file(input_mpg_file, &psdata, &size);

    Trace("psfile name=%s, size=%u", input_mpg_file.c_str(), size);

    if(!psdata || !size){
        Error("psdata is empty");
        return ret;
    }    
   
    if (!psdata || !size){
        Error("ps data is NULL!");
        return ret;
    }

    Trace("ps data %x, %x, %x, %x", psdata[0], psdata[1], psdata[2], psdata[3]);

    int pid = getpid();
    Trace("pid=%d", pid);

   
    
    // if count is zero, infinity loop.
    for(int i = 0; count == 0 || i < count; i++){
        statistic->OnTaskStart(GetId(), url.GetUrl());

        uint32_t ssrc = pid << 16 | GetId();
        Trace("pid=%d, ssrc=%u", pid, ssrc);
        client->publish_ps(psdata, size, 0, ssrc);
        
        
        // if((ret = client.Publish(input_flv_file, &url)) != ERROR_SUCCESS){
        //     statistic->OnTaskError(GetId(), 0);
            
        //     Error("rtmp client publish url failed. ret=%d", ret);
        //     st_usleep((st_utime_t)(error_seconds * 1000 * 1000));
        //     continue;
        // }
        
        int sleep_ms = StUtility::BuildRandomMTime((delay_seconds >= 0)? delay_seconds:0);
        Trace("[RTMP] %s publish success, sleep %dms", url.GetUrl(), sleep_ms);
        st_usleep(sleep_ms * 1000);
        
        statistic->OnTaskEnd(GetId(), 0);
    }
    
    return ret;
}

