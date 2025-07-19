/**
 * Use the Node Addon API to isolate csound.node from changes in the
 * underlying v8 API.
 */
 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <CsoundProducer.hpp>

// Null from ecl/cons.h conflicts with Null from napi.h.
#ifdef Null
#undef Null
#endif

#include <csound.h>
#include <csound_threaded.hpp>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <napi.h>
#include <string>
#include <vector>

static csound::CsoundProducer csound_;
static napi_threadsafe_function message_tsfn = nullptr;

static void message_(const char *text) {
    if (message_tsfn == nullptr) {
        fprintf(stderr, "csound.node: null message_tsfn.\n");
        return;
    }
    fprintf(stderr, "%s", text);
    napi_status status = napi_call_threadsafe_function(
        message_tsfn,
        strdup(text),
        napi_tsfn_nonblocking
    );
    if (status != napi_ok) {
        fprintf(stderr, "csound.node: message_tsfn call failed: %d\n", status);
    }
}

/**
 * As this will often be called from Csound's native performance thread, 
 * it is not safe to call from here back into JavaScript. Hence, we enqueue 
 * messages to be dequeued and dispatched from the main JavaScript thread.
 * Dispatching is implemented using napi_call_threadsafe_function.
 */
static void csoundMessageCallback_(CSOUND *csound__, int attr, const char *format, va_list valist)
{
    char buffer[0x2000];
    std::vsprintf(buffer, format, valist);
    message_(buffer);
}

Napi::Number Cleanup(const Napi::CallbackInfo &info) {
    message_("Info: jscsound: Cleanup.\n");
    Napi::Env env = info.Env();
    int result = csound_.Cleanup();
    return Napi::Number::New(env, result);
}

Napi::Number CompileCsd(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string csd = info[0].As<Napi::String>().Utf8Value();
    int result = csound_.CompileCsd(csd.c_str());
    return Napi::Number::New(env, result);
}

Napi::Number CompileCsdText(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string csd = info[0].As<Napi::String>().Utf8Value();
    int result = csound_.CompileCsdText(csd.c_str());
    return Napi::Number::New(env, result);
}

Napi::Number CompileOrc(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string csd = info[0].As<Napi::String>().Utf8Value();
    int result = csound_.CompileOrc(csd.c_str());
    return Napi::Number::New(env, result);
}

Napi::Number EvalCode(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string code = info[0].As<Napi::String>().Utf8Value();
    double value = csound_.EvalCode(code.c_str());
    return Napi::Number::New(env, value);
}

Napi::Number GetControlChannel(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string name = info[0].As<Napi::String>().Utf8Value();
    int result = 0;
    double value = csound_.GetControlChannel(name.c_str(), &result);
    return Napi::Number::New(env, value);
}

Napi::Boolean GetDoGitCommit(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    bool value = csound_.GetDoGitCommit();
    return Napi::Boolean::New(env, value);
}

Napi::Number GetKsmps(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int value = csound_.GetKsmps();
    return Napi::Number::New(env, value);
}

Napi::String GetMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string tag = info[0].As<Napi::String>().Utf8Value();
    std::string value = csound_.GetMetadata(tag.c_str());
    return Napi::String::New(env, value);
}

Napi::Number GetNchnls(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int value = csound_.GetNchnls();
    return Napi::Number::New(env, value);
}

Napi::Number GetScoreTime(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    double value = csound_.GetScoreTime();
    return Napi::Number::New(env, value);
}

Napi::Number GetSr(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int value = csound_.GetSr();
    return Napi::Number::New(env, value);
}

Napi::Number GetVersion(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int value = csound_.GetVersion();
    return Napi::Number::New(env, value);
}

void InputMessage(const Napi::CallbackInfo &info) {
    std::string text = info[0].As<Napi::String>().Utf8Value();
    csound_.InputMessage(text.c_str());
}

Napi::Boolean IsPlaying(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    bool value = csound_.IsPlaying();
    return Napi::Boolean::New(env, value);
}

Napi::Boolean IsScorePending(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    bool value = csound_.IsScorePending();
    return Napi::Boolean::New(env, value);
}

void Message(const Napi::CallbackInfo &info) {
    std::string text = info[0].As<Napi::String>().Utf8Value();
    csound_.Message(text.c_str());
}

Napi::Number Perform(const Napi::CallbackInfo &info) {
    message_("Info: jscsound: Perform.\n");
    Napi::Env env = info.Env();
    int result = csound_.Perform();
    return Napi::Number::New(env, result);
}

Napi::Number PerformAndPostProcess(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int result = csound_.PerformAndPostProcess();
    return Napi::Number::New(env, result);
}

Napi::Number ReadScore(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    std::string sco = info[0].As<Napi::String>().Utf8Value();
    int result = csound_.ReadScore(sco.c_str());
    return Napi::Number::New(env, result);
}

void Reset(const Napi::CallbackInfo &info) {
    message_("Info: jscsound: Reset.\n");
    csound_.Reset();
}

void RewindScore(const Napi::CallbackInfo &info) {
    csound_.RewindScore();
}

/**
 * Evaluates a single score event, sent as opcode and pfields,
 * relative to the current performance time. The number of pfields
 * is read from the length of the array, not from the Csound API
 * parameter.
 */
Napi::Number ScoreEvent(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::string opcode = info[0].As<Napi::String>().Utf8Value();
    Napi::Array array = info[1].As<Napi::Array>();
    std::vector<MYFLT> pfields;
    for (int i = 0, n = array.Length(); i < n; ++i) {
        Napi::Value value = array[i];
        pfields.push_back(value.As<Napi::Number>().DoubleValue());
    }
    int result = csound_.ScoreEvent(opcode[0], pfields.data(), pfields.size());
    return Napi::Number::New(env, result);
}

void SetControlChannel(const Napi::CallbackInfo &info) {
    std::string name = info[0].As<Napi::String>().Utf8Value();
    double value = info[1].As<Napi::Number>().DoubleValue();
    csound_.SetChannel(name.c_str(), value);
}

void SetDoGitCommit(const Napi::CallbackInfo &info) {
    bool value = info[0].As<Napi::Boolean>().Value();
    csound_.SetDoGitCommit(value);
}

void SetMessageCallback(const Napi::CallbackInfo& info) {
    fprintf(stderr, "csound.node: SetMessageCallback...\n");
    Napi::Env env = info.Env();
    Napi::Function jsCallback = info[0].As<Napi::Function>();
    napi_value fn = jsCallback;
    napi_create_threadsafe_function(
        env, fn, nullptr, 
        Napi::String::New(env, "csoundMessageCallback"), 
        0, 1, nullptr, nullptr, nullptr, 
        [](napi_env env, napi_value js_cb, void* context, void* data) {
            // This runs on the JS thread!
            char* message = static_cast<char*>(data);
            napi_value argv[1];
            napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &argv[0]);
            napi_call_function(env, nullptr, js_cb, 1, argv, nullptr);
            std::free(message);
        }, 
        &message_tsfn
    );
}

void SetMetadata(const Napi::CallbackInfo &info) {
    std::string tag = info[0].As<Napi::String>().Utf8Value();
    std::string value = info[1].As<Napi::String>().Utf8Value();
    csound_.SetMetadata(tag.c_str(), value.c_str());
}

void SetOption(const Napi::CallbackInfo &info) {
    std::string value = info[0].As<Napi::String>().Utf8Value();
    csound_.SetOption(value.c_str());
}

void SetOutput(const Napi::CallbackInfo &info) {
    std::string filename = info[0].As<Napi::String>().Utf8Value();
    std::string type = info[1].As<Napi::String>().Utf8Value();
    std::string format = info[2].As<Napi::String>().Utf8Value();
    csound_.SetOutput(filename.c_str(), type.c_str(), format.c_str());
}

void SetScorePending(const Napi::CallbackInfo &info) {
    bool value = info[0].As<Napi::Boolean>().Value();
    csound_.SetScorePending(value);
}

Napi::Number Start(const Napi::CallbackInfo &info) {
    message_("Info: jscsound: Start.\n");
    Napi::Env env = info.Env();
    int result = csound_.Start();
    if (result == -1) {
        message_("Warning: jscsound: Still playing? Try stopping and restarting.\n");
        csound_.Stop();
        csound_.Join();
        result = csound_.Start();
    }
    return Napi::Number::New(env, result);
}

void Stop(const Napi::CallbackInfo &info) {
    message_("Info: jscsound: Stop.\n");
    csound_.Stop();
    csound_.Join();
}

void on_exit() {
    if (message_tsfn != nullptr) {
        napi_release_threadsafe_function(message_tsfn, napi_tsfn_release);
        message_tsfn = nullptr;
    }
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    std::fprintf(stderr, "Initializing csound.node...\n");
    std::atexit(&on_exit);  
    csoundSetDefaultMessageCallback(csoundMessageCallback_);
    csound_.SetMessageCallback(csoundMessageCallback_);
    exports.Set(Napi::String::New(env, "Cleanup"),
                Napi::Function::New(env, Cleanup));
    exports.Set(Napi::String::New(env, "cleanup"),
                Napi::Function::New(env, Cleanup));
    exports.Set(Napi::String::New(env, "CompileCsd"),
                Napi::Function::New(env, CompileCsd));
    exports.Set(Napi::String::New(env, "compileCsd"),
                Napi::Function::New(env, CompileCsd));
    exports.Set(Napi::String::New(env, "CompileCsdText"),
                Napi::Function::New(env, CompileCsdText));
    exports.Set(Napi::String::New(env, "compileCsdText"),
                Napi::Function::New(env, CompileCsdText));
    exports.Set(Napi::String::New(env, "CompileOrc"),
                Napi::Function::New(env, CompileOrc));
    exports.Set(Napi::String::New(env, "compileOrc"),
                Napi::Function::New(env, CompileOrc));
    exports.Set(Napi::String::New(env, "EvalCode"),
                Napi::Function::New(env, EvalCode));
    exports.Set(Napi::String::New(env, "evalCode"),
                Napi::Function::New(env, EvalCode));
    exports.Set(Napi::String::New(env, "GetControlChannel"),
                Napi::Function::New(env, GetControlChannel));
    exports.Set(Napi::String::New(env, "getControlChannel"),
                Napi::Function::New(env, GetControlChannel));
    exports.Set(Napi::String::New(env, "GetKsmps"),
                Napi::Function::New(env, GetKsmps));
    exports.Set(Napi::String::New(env, "getKsmps"),
                Napi::Function::New(env, GetKsmps));
    exports.Set(Napi::String::New(env, "GetMetadata"),
                Napi::Function::New(env, GetMetadata));
    exports.Set(Napi::String::New(env, "getMetadata"),
                Napi::Function::New(env, GetMetadata));
    exports.Set(Napi::String::New(env, "GetNchnls"),
                Napi::Function::New(env, GetNchnls));
    exports.Set(Napi::String::New(env, "getNchnls"),
                Napi::Function::New(env, GetNchnls));
    exports.Set(Napi::String::New(env, "GetScoreTime"),
                Napi::Function::New(env, GetScoreTime));
    exports.Set(Napi::String::New(env, "getScoreTime"),
                Napi::Function::New(env, GetScoreTime));
    exports.Set(Napi::String::New(env, "GetVersion"),
                Napi::Function::New(env, GetVersion));
    exports.Set(Napi::String::New(env, "getVersion"),
                Napi::Function::New(env, GetVersion));
    exports.Set(Napi::String::New(env, "GetSr"),
                Napi::Function::New(env, GetSr));
    exports.Set(Napi::String::New(env, "getSr"),
                Napi::Function::New(env, GetSr));
    exports.Set(Napi::String::New(env, "InputMessage"),
                Napi::Function::New(env, InputMessage));
    exports.Set(Napi::String::New(env, "inputMessage"),
                Napi::Function::New(env, InputMessage));
    exports.Set(Napi::String::New(env, "IsPlaying"),
                Napi::Function::New(env, IsPlaying));
    exports.Set(Napi::String::New(env, "IsScorePending"),
                Napi::Function::New(env, IsScorePending));
    exports.Set(Napi::String::New(env, "isScorePending"),
                Napi::Function::New(env, IsScorePending));
    exports.Set(Napi::String::New(env, "Message"),
                Napi::Function::New(env, Message));
    exports.Set(Napi::String::New(env, "message"),
                Napi::Function::New(env, Message));
    exports.Set(Napi::String::New(env, "Perform"),
                Napi::Function::New(env, Perform));
    exports.Set(Napi::String::New(env, "perform"),
                Napi::Function::New(env, Perform));
    exports.Set(Napi::String::New(env, "PerformAndPostProcess"),
                Napi::Function::New(env, PerformAndPostProcess));
    exports.Set(Napi::String::New(env, "performAndPostProcess"),
                Napi::Function::New(env, PerformAndPostProcess));
    exports.Set(Napi::String::New(env, "ReadScore"),
                Napi::Function::New(env, ReadScore));
    exports.Set(Napi::String::New(env, "readScore"),
                Napi::Function::New(env, ReadScore));
    exports.Set(Napi::String::New(env, "Reset"),
                Napi::Function::New(env, Reset));
    exports.Set(Napi::String::New(env, "reset"),
                Napi::Function::New(env, Reset));
    exports.Set(Napi::String::New(env, "RewindScore"),
                Napi::Function::New(env, RewindScore));
    exports.Set(Napi::String::New(env, "rewindScore"),
                Napi::Function::New(env, RewindScore));
    exports.Set(Napi::String::New(env, "ScoreEvent"),
                Napi::Function::New(env, ScoreEvent));
    exports.Set(Napi::String::New(env, "scoreEvent"),
                Napi::Function::New(env, ScoreEvent));
    exports.Set(Napi::String::New(env, "SetControlChannel"),
                Napi::Function::New(env, SetControlChannel));
    exports.Set(Napi::String::New(env, "setControlChannel"),
                Napi::Function::New(env, SetControlChannel));
    exports.Set(Napi::String::New(env, "SetDoGitCommit"),
                Napi::Function::New(env, SetDoGitCommit));
    exports.Set(Napi::String::New(env, "setDoGitCommit"),
                Napi::Function::New(env, SetDoGitCommit));
    exports.Set(Napi::String::New(env, "SetMessageCallback"),
                Napi::Function::New(env, SetMessageCallback));
    exports.Set(Napi::String::New(env, "setMessageCallback"),
                Napi::Function::New(env, SetMessageCallback));
    exports.Set(Napi::String::New(env, "SetMetadata"),
                Napi::Function::New(env, SetMetadata));
    exports.Set(Napi::String::New(env, "setMetadata"),
                Napi::Function::New(env, SetMetadata));
    exports.Set(Napi::String::New(env, "SetOption"),
                Napi::Function::New(env, SetOption));
    exports.Set(Napi::String::New(env, "setOption"),
                Napi::Function::New(env, SetOption));
    exports.Set(Napi::String::New(env, "SetOutput"),
                Napi::Function::New(env, SetOutput));
    exports.Set(Napi::String::New(env, "setOutput"),
                Napi::Function::New(env, SetOutput));
    // exports.Set(Napi::String::New(env, "SetScorePendingOutput"),
    //             Napi::Function::New(env, SetScorePending));
    exports.Set(Napi::String::New(env, "setScorePending"),
                Napi::Function::New(env, SetScorePending));
    exports.Set(Napi::String::New(env, "Start"),
                Napi::Function::New(env, Start));
    exports.Set(Napi::String::New(env, "start"),
                Napi::Function::New(env, Start));
    exports.Set(Napi::String::New(env, "Stop"),
                Napi::Function::New(env, Stop));
    exports.Set(Napi::String::New(env, "stop"),
                Napi::Function::New(env, Stop));
    std::fprintf(stderr, "Csound.node initialized.\n");
    return exports;
}

NODE_API_MODULE(csound, Initialize)
