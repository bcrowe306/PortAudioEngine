
#include <cstdint>
#include <cmath>
#include <cassert>
#include <string>
#include <cstring>
#include <array>
#include <iostream>

//==============================================================================
/// Auto-generated C++ class for the 'sine' processor
///

#if ! (defined (__cplusplus) && (__cplusplus >= 201703L))
 #error "This code requires that your compiler is set to use C++17 or later!"
#endif

struct sine
{
    sine() = default;
    ~sine() = default;

    static constexpr std::string_view name = "sine";

    //==============================================================================
    using EndpointHandle = uint32_t;

    enum class EndpointType
    {
        stream,
        event,
        value
    };

    struct EndpointInfo
    {
        uint32_t handle;
        const char* name;
        EndpointType endpointType;
    };

    //==============================================================================
    static constexpr uint32_t numInputEndpoints  = 14;
    static constexpr uint32_t numOutputEndpoints = 3;

    static constexpr uint32_t maxFramesPerBlock  = 512;
    static constexpr uint32_t eventBufferSize    = 32;
    static constexpr uint32_t maxOutputEventSize = 8;
    static constexpr double   latency            = 0.000000;

    enum class EndpointHandles
    {
        waveform         = 1 ,
        gate             = 2 ,
        out              = 15,
        metronomeEvent   = 16,
        amount           = 3 ,
        tempo            = 4 ,
        timeSigNumerator = 5 ,
        denominator      = 6 ,
        threshold_dB     = 7 ,
        ratio            = 8 ,
        attack_ms        = 9 ,
        release_ms       = 10,
        makeup_db        = 11,
        softKnee         = 12,
        autoMakeup       = 13,
        bypass           = 14,
        console          = 17
    };

    static constexpr uint32_t getEndpointHandleForName (std::string_view endpointName)
    {
        if (endpointName == "waveform")          return static_cast<uint32_t> (EndpointHandles::waveform);
        if (endpointName == "gate")              return static_cast<uint32_t> (EndpointHandles::gate);
        if (endpointName == "out")               return static_cast<uint32_t> (EndpointHandles::out);
        if (endpointName == "metronomeEvent")    return static_cast<uint32_t> (EndpointHandles::metronomeEvent);
        if (endpointName == "amount")            return static_cast<uint32_t> (EndpointHandles::amount);
        if (endpointName == "tempo")             return static_cast<uint32_t> (EndpointHandles::tempo);
        if (endpointName == "timeSigNumerator")  return static_cast<uint32_t> (EndpointHandles::timeSigNumerator);
        if (endpointName == "denominator")       return static_cast<uint32_t> (EndpointHandles::denominator);
        if (endpointName == "threshold_dB")      return static_cast<uint32_t> (EndpointHandles::threshold_dB);
        if (endpointName == "ratio")             return static_cast<uint32_t> (EndpointHandles::ratio);
        if (endpointName == "attack_ms")         return static_cast<uint32_t> (EndpointHandles::attack_ms);
        if (endpointName == "release_ms")        return static_cast<uint32_t> (EndpointHandles::release_ms);
        if (endpointName == "makeup_db")         return static_cast<uint32_t> (EndpointHandles::makeup_db);
        if (endpointName == "softKnee")          return static_cast<uint32_t> (EndpointHandles::softKnee);
        if (endpointName == "autoMakeup")        return static_cast<uint32_t> (EndpointHandles::autoMakeup);
        if (endpointName == "bypass")            return static_cast<uint32_t> (EndpointHandles::bypass);
        if (endpointName == "console")           return static_cast<uint32_t> (EndpointHandles::console);
        return 0;
    }

    static constexpr EndpointInfo inputEndpoints[] =
    {
        { 1,   "waveform",          EndpointType::event },
        { 2,   "gate",              EndpointType::value },
        { 3,   "amount",            EndpointType::event },
        { 4,   "tempo",             EndpointType::value },
        { 5,   "timeSigNumerator",  EndpointType::value },
        { 6,   "denominator",       EndpointType::event },
        { 7,   "threshold_dB",      EndpointType::value },
        { 8,   "ratio",             EndpointType::value },
        { 9,   "attack_ms",         EndpointType::event },
        { 10,  "release_ms",        EndpointType::event },
        { 11,  "makeup_db",         EndpointType::value },
        { 12,  "softKnee",          EndpointType::value },
        { 13,  "autoMakeup",        EndpointType::value },
        { 14,  "bypass",            EndpointType::value }
    };

    static constexpr EndpointInfo outputEndpoints[] =
    {
        { 15,  "out",             EndpointType::stream },
        { 16,  "metronomeEvent",  EndpointType::stream },
        { 17,  "console",         EndpointType::event  }
    };

    //==============================================================================
    static constexpr uint32_t numAudioInputChannels  = 0;
    static constexpr uint32_t numAudioOutputChannels = 1;

    static constexpr std::array outputAudioStreams
    {
        outputEndpoints[0]
    };

    static constexpr std::array outputEvents
    {
        outputEndpoints[2]
    };

    static constexpr std::array<EndpointInfo, 0> outputMIDIEvents {};

    static constexpr std::array<EndpointInfo, 0> inputAudioStreams {};

    static constexpr std::array inputEvents
    {
        inputEndpoints[0],
        inputEndpoints[2],
        inputEndpoints[5],
        inputEndpoints[8],
        inputEndpoints[9]
    };

    static constexpr std::array<EndpointInfo, 0> inputMIDIEvents {};

    static constexpr std::array inputParameters
    {
        inputEndpoints[0],
        inputEndpoints[1],
        inputEndpoints[2],
        inputEndpoints[3],
        inputEndpoints[4],
        inputEndpoints[5],
        inputEndpoints[6],
        inputEndpoints[7],
        inputEndpoints[8],
        inputEndpoints[9],
        inputEndpoints[10],
        inputEndpoints[11],
        inputEndpoints[12],
        inputEndpoints[13]
    };

    static constexpr const char* programDetailsJSON =
            "{\n"
            "  \"mainProcessor\": \"sine\",\n"
            "  \"inputs\": [\n"
            "    {\n"
            "      \"endpointID\": \"waveform\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"int32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Waveform\",\n"
            "        \"init\": \"Sine\",\n"
            "        \"text\": \"sine | saw | square\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"gate\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"bool\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Gate\",\n"
            "        \"init\": false,\n"
            "        \"boolean\": \"true\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"amount\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"int32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Amount\",\n"
            "        \"min\": -100,\n"
            "        \"max\": 100,\n"
            "        \"init\": 0\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"tempo\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Tempo\",\n"
            "        \"init\": 120,\n"
            "        \"min\": 0.0,\n"
            "        \"max\": 900\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"timeSigNumerator\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"int32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Time Signature Numerator\",\n"
            "        \"init\": 4,\n"
            "        \"min\": 1,\n"
            "        \"max\": 16,\n"
            "        \"step\": 1\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"denominator\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"int32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Denominator\",\n"
            "        \"init\": 1,\n"
            "        \"text\": \"Half | Quarter | Eighth\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"threshold_dB\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Threshold\",\n"
            "        \"init\": -12,\n"
            "        \"min\": -60,\n"
            "        \"max\": 0.0\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"ratio\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Ratio\",\n"
            "        \"init\": 4,\n"
            "        \"min\": 1,\n"
            "        \"max\": 30\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"attack_ms\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Attack\",\n"
            "        \"init\": 3,\n"
            "        \"min\": 0.0,\n"
            "        \"max\": 50\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"release_ms\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Release\",\n"
            "        \"init\": 3,\n"
            "        \"min\": 0.0,\n"
            "        \"max\": 50\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"makeup_db\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Makeup Gain\",\n"
            "        \"init\": 0.0,\n"
            "        \"min\": -24,\n"
            "        \"max\": 24\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"softKnee\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"bool\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Soft Knee\",\n"
            "        \"init\": true,\n"
            "        \"boolean\": \"true\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"autoMakeup\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"bool\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Auto Makeup Gain\",\n"
            "        \"init\": false,\n"
            "        \"boolean\": \"true\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"bypass\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"bool\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Bypass\",\n"
            "        \"init\": false,\n"
            "        \"boolean\": \"true\"\n"
            "      },\n"
            "      \"purpose\": \"parameter\"\n"
            "    }\n"
            "  ],\n"
            "  \"outputs\": [\n"
            "    {\n"
            "      \"endpointID\": \"out\",\n"
            "      \"endpointType\": \"stream\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"purpose\": \"audio out\",\n"
            "      \"numAudioChannels\": 1\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"metronomeEvent\",\n"
            "      \"endpointType\": \"stream\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"int32\"\n"
            "      }\n"
            "    },\n"
            "    {\n"
            "      \"endpointID\": \"console\",\n"
            "      \"endpointType\": \"event\",\n"
            "      \"dataTypes\": [\n"
            "        {\n"
            "          \"type\": \"int32\"\n"
            "        },\n"
            "        {\n"
            "          \"type\": \"int32\"\n"
            "        }\n"
            "      ],\n"
            "      \"purpose\": \"console\"\n"
            "    }\n"
            "  ]\n"
            "}";

    //==============================================================================
    struct intrinsics;

    using SizeType = int32_t;
    using IndexType = int32_t;
    using StringHandle = uint32_t;

    struct Null
    {
        template <typename AnyType> operator AnyType() const    { return {}; }
        Null operator[] (IndexType) const                       { return {}; }
    };

    //==============================================================================
    template <typename ElementType, SizeType numElements>
    struct Array
    {
        Array() = default;
        Array (Null) {}
        Array (const Array&) = default;

        template <typename ElementOrList>
        Array (const ElementOrList& value) noexcept
        {
            if constexpr (std::is_convertible<ElementOrList, ElementType>::value)
            {
                for (IndexType i = 0; i < numElements; ++i)
                    this->elements[i] = static_cast<ElementType> (value);
            }
            else
            {
                for (IndexType i = 0; i < numElements; ++i)
                    this->elements[i] = static_cast<ElementType> (value[i]);
            }
        }

        template <typename... Others>
        Array (ElementType e0, ElementType e1, Others... others) noexcept
        {
            this->elements[0] = static_cast<ElementType> (e0);
            this->elements[1] = static_cast<ElementType> (e1);

            if constexpr (numElements > 2)
            {
                const ElementType initialisers[] = { static_cast<ElementType> (others)... };

                for (size_t i = 0; i < sizeof...(others); ++i)
                    this->elements[i + 2] = initialisers[i];
            }
        }

        Array (const ElementType* rawArray, size_t) noexcept
        {
            for (IndexType i = 0; i < numElements; ++i)
                this->elements[i] = rawArray[i];
        }

        Array& operator= (const Array&) noexcept = default;
        Array& operator= (Null) noexcept                 { this->clear(); return *this; }

        template <typename ElementOrList>
        Array& operator= (const ElementOrList& value) noexcept
        {
            if constexpr (std::is_convertible<ElementOrList, ElementType>::value)
            {
                for (IndexType i = 0; i < numElements; ++i)
                    this->elements[i] = static_cast<ElementType> (value);
            }
            else
            {
                for (IndexType i = 0; i < numElements; ++i)
                    this->elements[i] = static_cast<ElementType> (value[i]);
            }
        }

        static constexpr SizeType size()                                    { return numElements; }

        const ElementType& operator[] (IndexType index) const noexcept      { return this->elements[index]; }
        ElementType& operator[] (IndexType index) noexcept                  { return this->elements[index]; }

        void clear() noexcept
        {
            for (auto& element : elements)
                element = ElementType();
        }

        void clear (SizeType numElementsToClear) noexcept
        {
            for (SizeType i = 0; i < numElementsToClear; ++i)
                elements[i] = ElementType();
        }

        ElementType elements[numElements] = {};
    };

    //==============================================================================
    template <typename ElementType, SizeType numElements>
    struct Vector  : public Array<ElementType, numElements>
    {
        Vector() = default;
        Vector (Null) {}

        template <typename ElementOrList>
        Vector (const ElementOrList& value) noexcept  : Array<ElementType, numElements> (value) {}

        template <typename... Others>
        Vector (ElementType e0, ElementType e1, Others... others) noexcept  : Array<ElementType, numElements> (e0, e1, others...) {}

        Vector (const ElementType* rawArray, size_t) noexcept  : Array<ElementType, numElements> (rawArray, size_t()) {}

        template <typename ElementOrList>
        Vector& operator= (const ElementOrList& value) noexcept { return Array<ElementType, numElements>::operator= (value); }

        Vector& operator= (Null) noexcept { this->clear(); return *this; }

        operator ElementType() const noexcept
        {
            static_assert (numElements == 1);
            return this->elements[0];
        }

        constexpr auto operator!() const noexcept     { return performUnaryOp ([] (ElementType n) { return ! n; }); }
        constexpr auto operator~() const noexcept     { return performUnaryOp ([] (ElementType n) { return ~n; }); }
        constexpr auto operator-() const noexcept     { return performUnaryOp ([] (ElementType n) { return -n; }); }

        constexpr auto operator+ (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a + b; }); }
        constexpr auto operator- (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a - b; }); }
        constexpr auto operator* (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a * b; }); }
        constexpr auto operator/ (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a / b; }); }
        constexpr auto operator% (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return intrinsics::modulo (a, b); }); }
        constexpr auto operator& (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a & b; }); }
        constexpr auto operator| (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a | b; }); }
        constexpr auto operator<< (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a << b; }); }
        constexpr auto operator>> (const Vector& rhs) const noexcept   { return performBinaryOp (rhs, [] (ElementType a, ElementType b) { return a >> b; }); }

        constexpr auto operator== (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a == b; }); }
        constexpr auto operator!= (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a != b; }); }
        constexpr auto operator<  (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a < b; }); }
        constexpr auto operator<= (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a <= b; }); }
        constexpr auto operator>  (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a > b; }); }
        constexpr auto operator>= (const Vector& rhs) const noexcept  { return performComparison (rhs, [] (ElementType a, ElementType b) { return a >= b; }); }

        template <typename Functor>
        constexpr Vector performUnaryOp (Functor&& f) const noexcept
        {
            Vector result;

            for (IndexType i = 0; i < numElements; ++i)
                result.elements[i] = f (this->elements[i]);

            return result;
        }

        template <typename Functor>
        constexpr Vector performBinaryOp (const Vector& rhs, Functor&& f) const noexcept
        {
            Vector result;

            for (IndexType i = 0; i < numElements; ++i)
                result.elements[i] = f (this->elements[i], rhs.elements[i]);

            return result;
        }

        template <typename Functor>
        constexpr Vector<bool, numElements> performComparison (const Vector& rhs, Functor&& f) const noexcept
        {
            Vector<bool, numElements> result;

            for (IndexType i = 0; i < numElements; ++i)
                result.elements[i] = f (this->elements[i], rhs.elements[i]);

            return result;
        }
    };

    //==============================================================================
    template <typename ElementType>
    struct Slice
    {
        Slice() = default;
        Slice (Null) {}
        Slice (ElementType* e, SizeType size) : elements (e), numElements (size) {}
        Slice (const Slice&) = default;
        Slice& operator= (const Slice&) = default;
        template <typename ArrayType> Slice (const ArrayType& a) : elements (const_cast<ArrayType&> (a).elements), numElements (a.size()) {}
        template <typename ArrayType> Slice (const ArrayType& a, SizeType offset, SizeType size) : elements (const_cast<ArrayType&> (a).elements + offset), numElements (size) {}

        constexpr SizeType size() const                                     { return numElements; }
        ElementType operator[] (IndexType index) const noexcept             { return numElements == 0 ? ElementType() : elements[index]; }
        ElementType& operator[] (IndexType index) noexcept                  { return numElements == 0 ? emptyValue : elements[index]; }

        Slice slice (IndexType start, IndexType end) noexcept
        {
            if (numElements == 0) return {};
            if (start >= numElements) return {};

            return { elements + start, std::min (static_cast<SizeType> (end - start), numElements - start) };
        }

        ElementType* elements = nullptr;
        SizeType numElements = 0;

        static inline ElementType emptyValue {};
    };

    //==============================================================================
    #if __clang__
     #pragma clang diagnostic push
     #pragma clang diagnostic ignored "-Wunused-variable"
     #pragma clang diagnostic ignored "-Wunused-parameter"
     #pragma clang diagnostic ignored "-Wunused-label"
     #pragma clang diagnostic ignored "-Wtautological-compare"

     #if __clang_major__ >= 14
      #pragma clang diagnostic ignored "-Wunused-but-set-variable"
     #endif

    #elif __GNUC__
     #pragma GCC diagnostic push
     #pragma GCC diagnostic ignored "-Wunused-variable"
     #pragma GCC diagnostic ignored "-Wunused-parameter"
     #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
     #pragma GCC diagnostic ignored "-Wunused-label"
    #else
     #pragma warning (push, 0)
     #pragma warning (disable: 4702)
     #pragma warning (disable: 4706)
    #endif

    //==============================================================================
    struct std_oscillators_PhasorState
    {
        float phase = {};
        float increment = {};
    };

    struct std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_State
    {
        std_oscillators_PhasorState phasor;
        int32_t _resumeIndex = {};
    };

    struct _EnvelopeAttenuator_1_State
    {
        float scale = {};
        int32_t _resumeIndex = {};
    };

    struct _Time_1_State
    {
        Slice<int32_t> timeSigDenominators;
        int32_t timeSigDenominator = {};
        float sampleRate = {};
        int32_t counter = {};
        int32_t tick = {};
        int32_t beat = {};
        int32_t bar = {};
        int32_t metVal = {};
        float _v_tempo = {};
        int32_t _v_timeSigNumerator = {};
        int32_t _resumeIndex = {};
    };

    struct _Compressor_1_State
    {
        float envelope = {};
        float attackCoeff = {};
        float releaseCoeff = {};
        float _v_threshold_dB = {};
        float _v_ratio = {};
        float _v_makeup_db = {};
        bool _v_softKnee = {};
        bool _v_sideChain = {};
        bool _v_autoMakeup = {};
        bool _v_bypass = {};
        int32_t _resumeIndex = {};
    };

    struct _sine_State
    {
        int32_t _sessionID = {};
        double _frequency = {};
        std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_State osc;
        _EnvelopeAttenuator_1_State env_atten;
        _Time_1_State time;
        _Compressor_1_State compressor;
        bool _v_gate = {};
        float _v_tempo = {};
        int32_t _v_timeSigNumerator = {};
        float _v_threshold_dB = {};
        float _v_ratio = {};
        float _v_makeup_db = {};
        bool _v_softKnee = {};
        bool _v_autoMakeup = {};
        bool _v_bypass = {};
    };

    struct sine_value_tempo
    {
        float value = {};
        float increment = {};
        int32_t frames = {};
    };

    struct sine_value_threshold_dB
    {
        float value = {};
        float increment = {};
        int32_t frames = {};
    };

    struct sine_value_ratio
    {
        float value = {};
        float increment = {};
        int32_t frames = {};
    };

    struct sine_value_makeup_db
    {
        float value = {};
        float increment = {};
        int32_t frames = {};
    };

    struct sine_eventValue_console
    {
        int32_t frame = {};
        int32_t type = {};
        int32_t value_0;
        int32_t value_1;
    };

    struct sine_State
    {
        int32_t _activeRamps = {};
        int32_t _currentFrame = {};
        _sine_State _state;
        bool _v_gate = {};
        sine_value_tempo _v_tempo;
        int32_t _v_timeSigNumerator = {};
        sine_value_threshold_dB _v_threshold_dB;
        sine_value_ratio _v_ratio;
        sine_value_makeup_db _v_makeup_db;
        bool _v_softKnee = {};
        bool _v_autoMakeup = {};
        bool _v_bypass = {};
        int32_t console_eventCount = {};
        Array<sine_eventValue_console, 32> console;
    };

    struct sine_IO
    {
        Array<float, 512> out;
        Array<int32_t, 512> metronomeEvent;
    };

    struct _sine_IO
    {
        float out = {};
        int32_t metronomeEvent = {};
    };

    struct std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_IO
    {
        float out = {};
    };

    struct _EnvelopeAttenuator_1_IO
    {
        float in = {};
        float out = {};
    };

    struct _Time_1_IO
    {
        int32_t metronomeEvent = {};
    };

    struct _Compressor_1_IO
    {
        float out = {};
        float in = {};
        float sideChainIn = {};
    };

    using std_intrinsics_T = double;
    using std_intrinsics_T_0 = double;
    using std_intrinsics_T_1 = float;
    using std_intrinsics_T_2 = float;
    using std_levels_T = float;
    using std_intrinsics_T_3 = float;
    using std_levels_T_0 = float;
    using std_intrinsics_T_4 = float;

    //==============================================================================
    double getMaxFrequency() const
    {
        return 192000.0;
    }

    void initialise (int32_t sessionID, double frequency)
    {
        assert (frequency <= getMaxFrequency());
        initSessionID = sessionID;
        initFrequency = frequency;
        reset();
    }

    void reset()
    {
        std::memset (reinterpret_cast<char*> (&cmajState), 0, sizeof (cmajState));
        int32_t processorID = 0;
        _initialise (cmajState, processorID, initSessionID, initFrequency);
    }

    void advance (int32_t frames)
    {
        cmajIO.out.clear (static_cast<SizeType> (frames));
        cmajIO.metronomeEvent.clear (static_cast<SizeType> (frames));
        _advance (cmajState, cmajIO, frames);
    }

    void copyOutputValue (EndpointHandle endpointHandle, void* dest)
    {
        (void) endpointHandle; (void) dest;

        assert (false);
    }

    void copyOutputFrames (EndpointHandle endpointHandle, void* dest, uint32_t numFramesToCopy)
    {
        if (endpointHandle == 15) { std::memcpy (reinterpret_cast<char*> (dest), std::addressof (cmajIO.out), 4 * numFramesToCopy); std::memset (reinterpret_cast<char*> (std::addressof (cmajIO.out)), 0, 4 * numFramesToCopy); return; }
        if (endpointHandle == 16) { std::memcpy (reinterpret_cast<char*> (dest), std::addressof (cmajIO.metronomeEvent), 4 * numFramesToCopy); std::memset (reinterpret_cast<char*> (std::addressof (cmajIO.metronomeEvent)), 0, 4 * numFramesToCopy); return; }
        assert (false);
    }

    uint32_t getNumOutputEvents (EndpointHandle endpointHandle)
    {
        if (endpointHandle == 17) return (uint32_t) cmajState.console_eventCount;
        assert (false); return {};
    }

    void resetOutputEventCount (EndpointHandle endpointHandle)
    {
        if (endpointHandle == 17) { cmajState.console_eventCount = 0; return; }
    }

    uint32_t getOutputEventType (EndpointHandle endpointHandle, uint32_t index)
    {
        if (endpointHandle == 17) return (uint32_t) cmajState.console[(IndexType) index].type;
        assert (false); return {};
    }

    static uint32_t getOutputEventDataSize (EndpointHandle endpointHandle, uint32_t typeIndex)
    {
        (void) endpointHandle; (void) typeIndex;

        if (endpointHandle == 17 && typeIndex == 0) return 4;
        if (endpointHandle == 17 && typeIndex == 1) return 4;
        assert (false); return 0;
    }

    uint32_t readOutputEvent (EndpointHandle endpointHandle, uint32_t index, unsigned char* dest)
    {
        if (endpointHandle == 17)
        {
            auto& event = cmajState.console[(IndexType) index];
            if (event.type == 0)
            {
                memcpy (dest, std::addressof (event.value_0), 4);
                dest += 4;
                return (uint32_t) event.frame;
            }
            if (event.type == 1)
            {
                memcpy (dest, std::addressof (event.value_1), 4);
                dest += 4;
                return (uint32_t) event.frame;
            }
        }

        assert (false);
        return {};
    }

    void addEvent_waveform (const int32_t& event)
    {
        _sendEvent_waveform (cmajState, event);
    }

    void addEvent_amount (int32_t event)
    {
        _sendEvent_amount (cmajState, event);
    }

    void addEvent_denominator (const int32_t& event)
    {
        _sendEvent_denominator (cmajState, event);
    }

    void addEvent_attack_ms (float event)
    {
        _sendEvent_attack_ms (cmajState, event);
    }

    void addEvent_release_ms (float event)
    {
        _sendEvent_release_ms (cmajState, event);
    }

    void addEvent (EndpointHandle endpointHandle, uint32_t typeIndex, const unsigned char* eventData)
    {
        (void) endpointHandle; (void) typeIndex; (void) eventData;

        if (endpointHandle == 1)
        {
            int32_t value;
            memcpy (&value, eventData, 4);
            eventData += 4;
            return addEvent_waveform (value);
        }

        if (endpointHandle == 3)
        {
            int32_t value;
            memcpy (&value, eventData, 4);
            eventData += 4;
            return addEvent_amount (value);
        }

        if (endpointHandle == 6)
        {
            int32_t value;
            memcpy (&value, eventData, 4);
            eventData += 4;
            return addEvent_denominator (value);
        }

        if (endpointHandle == 9)
        {
            float value;
            memcpy (&value, eventData, 4);
            eventData += 4;
            return addEvent_attack_ms (value);
        }

        if (endpointHandle == 10)
        {
            float value;
            memcpy (&value, eventData, 4);
            eventData += 4;
            return addEvent_release_ms (value);
        }
    }

    void setValue (EndpointHandle endpointHandle, const void* value, int32_t frames)
    {
        if (endpointHandle == 2) return _setValue_gate (cmajState, *(bool*) value, frames);
        if (endpointHandle == 4) return _setValue_tempo (cmajState, *(float*) value, frames);
        if (endpointHandle == 5) return _setValue_timeSigNumerator (cmajState, *(int32_t*) value, frames);
        if (endpointHandle == 7) return _setValue_threshold_dB (cmajState, *(float*) value, frames);
        if (endpointHandle == 8) return _setValue_ratio (cmajState, *(float*) value, frames);
        if (endpointHandle == 11) return _setValue_makeup_db (cmajState, *(float*) value, frames);
        if (endpointHandle == 12) return _setValue_softKnee (cmajState, *(bool*) value, frames);
        if (endpointHandle == 13) return _setValue_autoMakeup (cmajState, *(bool*) value, frames);
        if (endpointHandle == 14) return _setValue_bypass (cmajState, *(bool*) value, frames);
    }

    void setInputFrames (EndpointHandle endpointHandle, const void* frameData, uint32_t numFrames, uint32_t numTrailingFramesToClear)
    {
        (void) endpointHandle; (void) frameData; (void) numFrames; (void) numTrailingFramesToClear;
    }

    //==============================================================================
    // Rendering state values
    int32_t initSessionID;
    double initFrequency;
    sine_State cmajState = {};
    sine_IO cmajIO = {};

    //==============================================================================
    void _sendEvent_waveform (sine_State& _state, int32_t value) noexcept
    {
        _sine__waveform (_state._state, value);
    }

    void _sine__waveform (_sine_State& _state, int32_t waveform) noexcept
    {
        _sine___writeEvent_console (_state, waveform);
    }

    void _sine___writeEvent_console (_sine_State& _state, const int32_t& value) noexcept
    {
        sine___writeEvent_console (state_upcast_struct_sine_State_struct__fGRIcb(_state), value);
    }

    void sine___writeEvent_console (sine_State& _state, const int32_t& value) noexcept
    {
        sine___writeEvent_console_0 (_state, value);
    }

    void sine___writeEvent_console_0 (sine_State& _state, const int32_t& value) noexcept
    {
        if (_state.console_eventCount < int32_t {32})
        {
            _state.console[_state.console_eventCount].frame = _state._currentFrame;
            _state.console[_state.console_eventCount].type = int32_t {1};
            _state.console[_state.console_eventCount].value_1 = value;
        }
        ++_state.console_eventCount;
    }

    void _sendEvent_amount (sine_State& _state, int32_t value) noexcept
    {
        _sine__amount (_state._state, value);
    }

    void _sine__amount (_sine_State& _state, int32_t value) noexcept
    {
        EnvelopeAttenuator__amount (_state.env_atten, value);
    }

    void EnvelopeAttenuator__amount (_EnvelopeAttenuator_1_State& _state, int32_t amount) noexcept
    {
        _state.scale = (1.0f + (static_cast<float> (amount) / 100.0f));
    }

    void _sendEvent_denominator (sine_State& _state, int32_t value) noexcept
    {
        _sine__denominator (_state._state, value);
    }

    void _sine__denominator (_sine_State& _state, int32_t value) noexcept
    {
        Time__denominator (_state.time, value);
    }

    void Time__denominator (_Time_1_State& _state, int32_t denominator) noexcept
    {
        Time___writeEvent_console (_state, denominator);
        if (denominator == int32_t {0})
        {
            _state.timeSigDenominator = int32_t {2};
        }
        else
        {
            if (denominator == int32_t {1})
            {
                _state.timeSigDenominator = int32_t {4};
            }
            else
            {
                if (denominator == int32_t {2})
                {
                    _state.timeSigDenominator = int32_t {8};
                }
            }
        }
    }

    void Time___writeEvent_console (_Time_1_State& _state, const int32_t& value) noexcept
    {
        _sine___forwardEvent (state_upcast_struct_sine_State_struct_X5e4pb(_state), value);
    }

    void _sine___forwardEvent (_sine_State& _state, int32_t value) noexcept
    {
        _sine___writeEvent_console_0 (_state, value);
    }

    void _sine___writeEvent_console_0 (_sine_State& _state, const int32_t& value) noexcept
    {
        sine___writeEvent_console_1 (state_upcast_struct_sine_State_struct__fGRIcb(_state), value);
    }

    void sine___writeEvent_console_1 (sine_State& _state, const int32_t& value) noexcept
    {
        sine___writeEvent_console_2 (_state, value);
    }

    void sine___writeEvent_console_2 (sine_State& _state, const int32_t& value) noexcept
    {
        if (_state.console_eventCount < int32_t {32})
        {
            _state.console[_state.console_eventCount].frame = _state._currentFrame;
            _state.console[_state.console_eventCount].type = int32_t {0};
            _state.console[_state.console_eventCount].value_0 = value;
        }
        ++_state.console_eventCount;
    }

    void _sendEvent_attack_ms (sine_State& _state, float value) noexcept
    {
        _sine__attack_ms (_state._state, value);
    }

    void _sine__attack_ms (_sine_State& _state, float value) noexcept
    {
        Compressor__attack_ms (_state.compressor, value);
    }

    void Compressor__attack_ms (_Compressor_1_State& _state, float attack_ms) noexcept
    {
        _state.attackCoeff = static_cast<float> (intrinsics::exp (static_cast<double> (-1.0f) / ((static_cast<double> (attack_ms) * (1.0 * g__frequency)) * static_cast<double> (0.001f))));
    }

    double std__intrinsics__exp (double n) noexcept
    {
        {
            return 0.0;
        }
    }

    void _sendEvent_release_ms (sine_State& _state, float value) noexcept
    {
        _sine__release_ms (_state._state, value);
    }

    void _sine__release_ms (_sine_State& _state, float value) noexcept
    {
        Compressor__release_ms (_state.compressor, value);
    }

    void Compressor__release_ms (_Compressor_1_State& _state, float release_ms) noexcept
    {
        _state.releaseCoeff = static_cast<float> (intrinsics::exp (static_cast<double> (-1.0f) / ((static_cast<double> (release_ms) * (1.0 * g__frequency)) * static_cast<double> (0.001f))));
    }

    void _setValue_gate (sine_State& _state, bool& value, int32_t frames) noexcept
    {
        _state._v_gate = value;
    }

    void _setValue_tempo (sine_State& _state, float& value, int32_t frames) noexcept
    {
        if (frames == int32_t {0})
        {
            frames = int32_t {1};
        }
        if (_state._v_tempo.frames == int32_t {0})
        {
            ++_state._activeRamps;
        }
        _state._v_tempo.increment = ((value - _state._v_tempo.value) / static_cast<float> (frames));
        _state._v_tempo.frames = frames;
    }

    void _setValue_timeSigNumerator (sine_State& _state, int32_t& value, int32_t frames) noexcept
    {
        _state._v_timeSigNumerator = value;
    }

    void _setValue_threshold_dB (sine_State& _state, float& value, int32_t frames) noexcept
    {
        if (frames == int32_t {0})
        {
            frames = int32_t {1};
        }
        if (_state._v_threshold_dB.frames == int32_t {0})
        {
            ++_state._activeRamps;
        }
        _state._v_threshold_dB.increment = ((value - _state._v_threshold_dB.value) / static_cast<float> (frames));
        _state._v_threshold_dB.frames = frames;
    }

    void _setValue_ratio (sine_State& _state, float& value, int32_t frames) noexcept
    {
        if (frames == int32_t {0})
        {
            frames = int32_t {1};
        }
        if (_state._v_ratio.frames == int32_t {0})
        {
            ++_state._activeRamps;
        }
        _state._v_ratio.increment = ((value - _state._v_ratio.value) / static_cast<float> (frames));
        _state._v_ratio.frames = frames;
    }

    void _setValue_makeup_db (sine_State& _state, float& value, int32_t frames) noexcept
    {
        if (frames == int32_t {0})
        {
            frames = int32_t {1};
        }
        if (_state._v_makeup_db.frames == int32_t {0})
        {
            ++_state._activeRamps;
        }
        _state._v_makeup_db.increment = ((value - _state._v_makeup_db.value) / static_cast<float> (frames));
        _state._v_makeup_db.frames = frames;
    }

    void _setValue_softKnee (sine_State& _state, bool& value, int32_t frames) noexcept
    {
        _state._v_softKnee = value;
    }

    void _setValue_autoMakeup (sine_State& _state, bool& value, int32_t frames) noexcept
    {
        _state._v_autoMakeup = value;
    }

    void _setValue_bypass (sine_State& _state, bool& value, int32_t frames) noexcept
    {
        _state._v_bypass = value;
    }

    void _initialise (sine_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        _sine___initialise (_state._state, processorID, sessionID, frequency);
    }

    void _sine___initialise (_sine_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        g__sessionID = sessionID;
        g__frequency = frequency;
        std__oscillators__Sine___initialise (_state.osc, processorID, sessionID, frequency);
        EnvelopeAttenuator___initialise (_state.env_atten, processorID, sessionID, frequency);
        Time___initialise (_state.time, processorID, sessionID, frequency);
        Compressor___initialise (_state.compressor, processorID, sessionID, frequency);
    }

    void std__oscillators__Sine___initialise (std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        g__sessionID = sessionID;
        g__frequency = frequency;
        std__oscillators__Sine__init (_state);
    }

    void std__oscillators__Sine__init (std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_State& _state) noexcept
    {
        std__oscillators__setFrequency (_state.phasor, 1.0 * g__frequency, 440.0);
    }

    void std__oscillators__setFrequency (std_oscillators_PhasorState& this_, double outputFrequencyHz, double oscillatorFrequencyHz) noexcept
    {
        this_.increment = static_cast<float> (intrinsics::fmod (oscillatorFrequencyHz / outputFrequencyHz, 1.0));
    }

    double std__intrinsics__fmod (double x, double y) noexcept
    {
        {
            return x - (y * static_cast<double> (static_cast<int64_t> (x / y)));
        }
    }

    void EnvelopeAttenuator___initialise (_EnvelopeAttenuator_1_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        g__sessionID = sessionID;
        g__frequency = frequency;
        _state.scale = 1.0f;
    }

    void Time___initialise (_Time_1_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        Array<int32_t, 3>  _temp;

        g__sessionID = sessionID;
        g__frequency = frequency;
        _state.timeSigDenominator = int32_t {4};
        _state.sampleRate = static_cast<float> (1.0 * g__frequency);
        _state.counter = int32_t {0};
        _state.tick = int32_t {0};
        _state.beat = int32_t {0};
        _state.bar = int32_t {0};
        _state.metVal = int32_t {0};
        _temp = Array<int32_t, 3> { int32_t {2}, int32_t {4}, int32_t {8} };
        _state.timeSigDenominators = Slice<int32_t> (_temp, 0, 3);
    }

    void Compressor___initialise (_Compressor_1_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        g__sessionID = sessionID;
        g__frequency = frequency;
        _state.envelope = 0.0f;
    }

    void _advance (sine_State& _state, sine_IO& _io, int32_t _frames) noexcept
    {
        _sine_IO  ioCopy;

        for (;;)
        {
            if (_state._currentFrame == _frames)
            {
                break;
            }
            if (_state._activeRamps != int32_t {0})
            {
                sine___updateRamps (_state);
                _state._state._v_tempo = _state._v_tempo.value;
                _state._state._v_threshold_dB = _state._v_threshold_dB.value;
                _state._state._v_ratio = _state._v_ratio.value;
                _state._state._v_makeup_db = _state._v_makeup_db.value;
            }
            _state._state._v_gate = _state._v_gate;
            _state._state._v_timeSigNumerator = _state._v_timeSigNumerator;
            _state._state._v_softKnee = _state._v_softKnee;
            _state._state._v_autoMakeup = _state._v_autoMakeup;
            _state._state._v_bypass = _state._v_bypass;
            ioCopy = _sine_IO {};
            main (_state._state, ioCopy);
            _io.out[_state._currentFrame] = ioCopy.out;
            _io.metronomeEvent[_state._currentFrame] = ioCopy.metronomeEvent;
            ++_state._currentFrame;
        }
        _state._currentFrame = int32_t {0};
    }

    void sine___updateRamps (sine_State& _state) noexcept
    {
        if (_state._v_tempo.frames != int32_t {0})
        {
            _state._v_tempo.value = (_state._v_tempo.value + _state._v_tempo.increment);
            --_state._v_tempo.frames;
            if (_state._v_tempo.frames == int32_t {0})
            {
                --_state._activeRamps;
            }
        }
        if (_state._v_threshold_dB.frames != int32_t {0})
        {
            _state._v_threshold_dB.value = (_state._v_threshold_dB.value + _state._v_threshold_dB.increment);
            --_state._v_threshold_dB.frames;
            if (_state._v_threshold_dB.frames == int32_t {0})
            {
                --_state._activeRamps;
            }
        }
        if (_state._v_ratio.frames != int32_t {0})
        {
            _state._v_ratio.value = (_state._v_ratio.value + _state._v_ratio.increment);
            --_state._v_ratio.frames;
            if (_state._v_ratio.frames == int32_t {0})
            {
                --_state._activeRamps;
            }
        }
        if (_state._v_makeup_db.frames != int32_t {0})
        {
            _state._v_makeup_db.value = (_state._v_makeup_db.value + _state._v_makeup_db.increment);
            --_state._v_makeup_db.frames;
            if (_state._v_makeup_db.frames == int32_t {0})
            {
                --_state._activeRamps;
            }
        }
    }

    void main (_sine_State& _state, _sine_IO& _io) noexcept
    {
        std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_IO  osc_io;
        _EnvelopeAttenuator_1_IO  env_atten_io;
        _Time_1_IO  time_io;
        _Compressor_1_IO  compressor_io;

        osc_io = std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_IO {};
        env_atten_io = _EnvelopeAttenuator_1_IO {};
        time_io = _Time_1_IO {};
        compressor_io = _Compressor_1_IO {};
        {
        }
        std__oscillators__Sine__main (_state.osc, osc_io);
        {
            _state.time._v_tempo = _state._v_tempo;
            _state.time._v_timeSigNumerator = _state._v_timeSigNumerator;
        }
        Time__main (_state.time, time_io);
        {
            env_atten_io.in = (env_atten_io.in + (osc_io.out * static_cast<float> (time_io.metronomeEvent)));
        }
        EnvelopeAttenuator__main (_state.env_atten, env_atten_io);
        {
            compressor_io.in = (compressor_io.in + env_atten_io.out);
            _state.compressor._v_threshold_dB = _state._v_threshold_dB;
            _state.compressor._v_ratio = _state._v_ratio;
            _state.compressor._v_makeup_db = _state._v_makeup_db;
            _state.compressor._v_softKnee = _state._v_softKnee;
            _state.compressor._v_autoMakeup = _state._v_autoMakeup;
            _state.compressor._v_bypass = _state._v_bypass;
        }
        Compressor__main (_state.compressor, compressor_io);
        {
            _io.out = (_io.out + compressor_io.out);
            _io.metronomeEvent = (_io.metronomeEvent + time_io.metronomeEvent);
        }
    }

    void std__oscillators__Sine__main (std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_State& _state, std_oscillators_std_oscillators_Sine_specialised_f32_4_yFnX4c_1_IO& _io) noexcept
    {
        for (;;)
        {
            _io.out = (_io.out + intrinsics::sin (std__oscillators__next (_state.phasor) * 6.2831855f));
            return;
        }
    }

    float std__intrinsics__sin (float n) noexcept
    {
        {
            return 0.0f;
        }
    }

    float std__oscillators__next (std_oscillators_PhasorState& this_) noexcept
    {
        float  p;
        float  next;

        p = this_.phase;
        next = p + this_.increment;
        if (next >= static_cast<float> (int32_t {1}))
        {
            next = (next - static_cast<float> (int32_t {1}));
        }
        this_.phase = next;
        return p;
    }

    void Time__main (_Time_1_State& _state, _Time_1_IO& _io) noexcept
    {
        float  ticksPerBeat;
        float  samplesPerTick;

        switch (_state._resumeIndex)
        {
            case 1: goto _branch0_0;
            default: break;
        }

        for (;;)
        {
            ticksPerBeat = g_ticksPerQuaterNote * (4.0f / static_cast<float> (_state.timeSigDenominator));
            samplesPerTick = ((_state.sampleRate * 60.0f) / _state._v_tempo) / g_ticksPerQuaterNote;
            if (static_cast<float> (_state.counter) >= samplesPerTick)
            {
                ++_state.tick;
                Time___writeEvent_tickEvent (_state, _state.tick);
                _state.counter = int32_t {0};
                if ((intrinsics::fmod (static_cast<float> (_state.tick), ticksPerBeat) == static_cast<float> (int32_t {0})) ? (_state.tick != int32_t {0}) : false)
                {
                    if ((intrinsics::modulo (_state.beat, _state._v_timeSigNumerator) == int32_t {0}) ? (_state.beat != int32_t {0}) : false)
                    {
                        Time___writeEvent_barEvent (_state, _state.bar);
                        _state.metVal = int32_t {1};
                        ++_state.bar;
                    }
                    else
                    {
                        Time___writeEvent_beatEvent (_state, _state.beat);
                        _state.metVal = int32_t {1};
                    }
                    ++_state.beat;
                }
                else
                {
                    auto _tempArg_0 = static_cast<float> (_state.tick);
                    if ((intrinsics::fmod (_tempArg_0, ticksPerBeat / static_cast<float> (int32_t {2})) == static_cast<float> (int32_t {0})) ? (_state.tick != int32_t {0}) : false)
                    {
                        _state.metVal = int32_t {0};
                    }
                    else
                    {
                        auto _tempArg_1 = static_cast<float> (_state.tick);
                        if ((intrinsics::fmod (_tempArg_1, ticksPerBeat / static_cast<float> (int32_t {4})) == static_cast<float> (int32_t {0})) ? (_state.tick != int32_t {0}) : false)
                        {
                            _state.metVal = int32_t {0};
                        }
                    }
                }
            }
            _io.metronomeEvent = (_io.metronomeEvent + _state.metVal);
            _state._resumeIndex = int32_t {1};
            return;
            _branch0_0:
            {
            }
            ++_state.counter;
        }
    }

    void Time___writeEvent_tickEvent (_Time_1_State& _state, int32_t value) noexcept
    {
    }

    float std__intrinsics__fmod_0 (float x, float y) noexcept
    {
        {
            return x - (y * static_cast<float> (static_cast<int64_t> (x / y)));
        }
    }

    void Time___writeEvent_barEvent (_Time_1_State& _state, int32_t value) noexcept
    {
    }

    void Time___writeEvent_beatEvent (_Time_1_State& _state, int32_t value) noexcept
    {
    }

    void EnvelopeAttenuator__main (_EnvelopeAttenuator_1_State& _state, _EnvelopeAttenuator_1_IO& _io) noexcept
    {
        for (;;)
        {
            _io.out = (_io.out + (_io.in * _state.scale));
            return;
        }
    }

    void Compressor__main (_Compressor_1_State& _state, _Compressor_1_IO& _io) noexcept
    {
        float  input_dB;
        float  gain_dB;
        float  makeup;
        float  totalGain_dB;
        float  linearGain;

        for (;;)
        {
            if (_state._v_bypass)
            {
                _io.out = (_io.out + _io.in);
            }
            else
            {
                input_dB = {};
                if (_state._v_sideChain)
                {
                    input_dB = std__levels__gainTodB (_io.sideChainIn);
                }
                else
                {
                    input_dB = std__levels__gainTodB (_io.in);
                }
                gain_dB = Compressor__computeGainReduction (_state, input_dB);
                _state.envelope = Compressor__smoothEnvelope (_state, gain_dB);
                makeup = {};
                if (_state._v_autoMakeup)
                {
                    makeup = Compressor__computeAutoMakeupGain (input_dB, _state.envelope);
                }
                else
                {
                    makeup = _state._v_makeup_db;
                }
                totalGain_dB = _state.envelope + makeup;
                linearGain = std__levels__dBtoGain (totalGain_dB);
                _io.out = (_io.out + (_io.in * linearGain));
            }
            return;
        }
    }

    float std__levels__gainTodB (float gainFactor) noexcept
    {
        return (gainFactor > static_cast<float> (int32_t {0})) ? (intrinsics::log10 (gainFactor) * 20.0f) : -100.0f;
    }

    float std__intrinsics__log10 (float n) noexcept
    {
        {
            return 0.0f;
        }
    }

    float Compressor__computeGainReduction (_Compressor_1_State& _state, float input_dB) noexcept
    {
        float  gain_dB;
        float  kneeWidth;
        float  kneeStart;
        float  kneeEnd;
        float  x;

        gain_dB = 0.0f;
        if (_state._v_softKnee)
        {
            kneeWidth = 6.0f;
            kneeStart = _state._v_threshold_dB - (kneeWidth / 2.0f);
            kneeEnd = _state._v_threshold_dB + (kneeWidth / 2.0f);
            if (input_dB > kneeStart)
            {
                if (input_dB < kneeEnd)
                {
                    x = (input_dB - kneeStart) / kneeWidth;
                    gain_dB = (_state._v_threshold_dB + ((x * (input_dB - _state._v_threshold_dB)) / _state._v_ratio));
                }
                else
                {
                    gain_dB = (_state._v_threshold_dB + ((input_dB - _state._v_threshold_dB) / _state._v_ratio));
                }
            }
            else
            {
                gain_dB = input_dB;
            }
        }
        else
        {
            if (input_dB > _state._v_threshold_dB)
            {
                gain_dB = (_state._v_threshold_dB + ((input_dB - _state._v_threshold_dB) / _state._v_ratio));
            }
            else
            {
                gain_dB = input_dB;
            }
        }
        return gain_dB - input_dB;
    }

    float Compressor__smoothEnvelope (_Compressor_1_State& _state, float gain_dB) noexcept
    {
        if (gain_dB < _state.envelope)
        {
            return (_state.attackCoeff * _state.envelope) + ((1.0f - _state.attackCoeff) * gain_dB);
        }
        else
        {
            return (_state.releaseCoeff * _state.envelope) + ((1.0f - _state.releaseCoeff) * gain_dB);
        }
    }

    float Compressor__computeAutoMakeupGain (float start_db, float end_db) noexcept
    {
        if (start_db < end_db)
        {
            return 0.0f;
        }
        else
        {
            return end_db - start_db;
        }
    }

    float std__levels__dBtoGain (float decibels) noexcept
    {
        return (decibels > -100.0f) ? intrinsics::pow (10.0f, decibels * 0.05f) : 0.0f;
    }

    float std__intrinsics__pow (float a, float b) noexcept
    {
        {
            return 0.0f;
        }
    }

    //==============================================================================
    const char* getStringForHandle (uint32_t handle, size_t& stringLength)
    {
        (void) handle; (void) stringLength;
        return "";
    }

    //==============================================================================
    #define OFFSETOF(type, member)   ((size_t)((char *)&(*(type *)nullptr).member))
    static _sine_State& state_upcast_struct_sine_State_struct_X5e4pb (_Time_1_State& child) { return *reinterpret_cast<_sine_State*> (reinterpret_cast<char*> (std::addressof (child)) - (OFFSETOF (_sine_State, time))); }
    static sine_State& state_upcast_struct_sine_State_struct__fGRIcb (_sine_State& child) { return *reinterpret_cast<sine_State*> (reinterpret_cast<char*> (std::addressof (child)) - (OFFSETOF (sine_State, _state))); }

    //==============================================================================
    double g__frequency {};
    int32_t g__sessionID {};
    static constexpr float g_ticksPerQuaterNote { 960.0f };

    //==============================================================================
    struct intrinsics
    {
        template <typename T> static T modulo (T a, T b)
        {
            if constexpr (std::is_floating_point<T>::value)
                return std::fmod (a, b);
            else
                return a % b;
        }

        template <typename T> static T addModulo2Pi (T a, T b)
        {
            constexpr auto twoPi = static_cast<T> (3.141592653589793238 * 2);
            auto n = a + b;
            return n >= twoPi ? std::remainder (n, twoPi) : n;
        }

        template <typename T> static T abs           (T a)              { return std::abs (a); }
        template <typename T> static T min           (T a, T b)         { return std::min (a, b); }
        template <typename T> static T max           (T a, T b)         { return std::max (a, b); }
        template <typename T> static T clamp         (T a, T b, T c)    { return a < b ? b : (a > c ? c : a); }
        template <typename T> static T wrap          (T a, T b)         { if (b == 0) return 0; auto n = modulo (a, b); if (n < 0) n += b; return n; }
        template <typename T> static T fmod          (T a, T b)         { return b != 0 ? std::fmod (a, b) : 0; }
        template <typename T> static T remainder     (T a, T b)         { return b != 0 ? std::remainder (a, b) : 0; }
        template <typename T> static T floor         (T a)              { return std::floor (a); }
        template <typename T> static T ceil          (T a)              { return std::ceil (a); }
        template <typename T> static T rint          (T a)              { return std::rint (a); }
        template <typename T> static T sqrt          (T a)              { return std::sqrt (a); }
        template <typename T> static T pow           (T a, T b)         { return std::pow (a, b); }
        template <typename T> static T exp           (T a)              { return std::exp (a); }
        template <typename T> static T log           (T a)              { return std::log (a); }
        template <typename T> static T log10         (T a)              { return std::log10 (a); }
        template <typename T> static T sin           (T a)              { return std::sin (a); }
        template <typename T> static T cos           (T a)              { return std::cos (a); }
        template <typename T> static T tan           (T a)              { return std::tan (a); }
        template <typename T> static T sinh          (T a)              { return std::sinh (a); }
        template <typename T> static T cosh          (T a)              { return std::cosh (a); }
        template <typename T> static T tanh          (T a)              { return std::tanh (a); }
        template <typename T> static T asinh         (T a)              { return std::asinh (a); }
        template <typename T> static T acosh         (T a)              { return std::acosh (a); }
        template <typename T> static T atanh         (T a)              { return std::atanh (a); }
        template <typename T> static T asin          (T a)              { return std::asin (a); }
        template <typename T> static T acos          (T a)              { return std::acos (a); }
        template <typename T> static T atan          (T a)              { return std::atan (a); }
        template <typename T> static T atan2         (T a, T b)         { return std::atan2 (a, b); }
        template <typename T> static T isnan         (T a)              { return std::isnan (a) ? 1 : 0; }
        template <typename T> static T isinf         (T a)              { return std::isinf (a) ? 1 : 0; }
        template <typename T> static T select        (bool c, T a, T b) { return c ? a : b; }

        static int32_t reinterpretFloatToInt (float   a)                { int32_t i; memcpy (std::addressof(i), std::addressof(a), sizeof(i)); return i; }
        static int64_t reinterpretFloatToInt (double  a)                { int64_t i; memcpy (std::addressof(i), std::addressof(a), sizeof(i)); return i; }
        static float   reinterpretIntToFloat (int32_t a)                { float   f; memcpy (std::addressof(f), std::addressof(a), sizeof(f)); return f; }
        static double  reinterpretIntToFloat (int64_t a)                { double  f; memcpy (std::addressof(f), std::addressof(a), sizeof(f)); return f; }

        static int32_t rightShiftUnsigned (int32_t a, int32_t b)        { return static_cast<int32_t> (static_cast<uint32_t> (a) >> b); }
        static int64_t rightShiftUnsigned (int64_t a, int64_t b)        { return static_cast<int64_t> (static_cast<uint64_t> (a) >> b); }

        struct VectorOps
        {
            template <typename Vec> static Vec abs     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::abs (x); }); }
            template <typename Vec> static Vec min     (Vec a, Vec b)     { return a.performBinaryOp (b, [] (auto x, auto y) { return intrinsics::min (x, y); }); }
            template <typename Vec> static Vec max     (Vec a, Vec b)     { return a.performBinaryOp (b, [] (auto x, auto y) { return intrinsics::max (x, y); }); }
            template <typename Vec> static Vec sqrt    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::sqrt (x); }); }
            template <typename Vec> static Vec log     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::log (x); }); }
            template <typename Vec> static Vec log10   (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::log10 (x); }); }
            template <typename Vec> static Vec sin     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::sin (x); }); }
            template <typename Vec> static Vec cos     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::cos (x); }); }
            template <typename Vec> static Vec tan     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::tan (x); }); }
            template <typename Vec> static Vec sinh    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::sinh (x); }); }
            template <typename Vec> static Vec cosh    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::cosh (x); }); }
            template <typename Vec> static Vec tanh    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::tanh (x); }); }
            template <typename Vec> static Vec asinh   (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::asinh (x); }); }
            template <typename Vec> static Vec acosh   (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::acosh (x); }); }
            template <typename Vec> static Vec atanh   (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::atanh (x); }); }
            template <typename Vec> static Vec asin    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::asin (x); }); }
            template <typename Vec> static Vec acos    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::acos (x); }); }
            template <typename Vec> static Vec atan    (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::atan (x); }); }
            template <typename Vec> static Vec atan2   (Vec a, Vec b)     { return a.performBinaryOp (b, [] (auto x, auto y) { return intrinsics::atan2 (x, y); }); }
            template <typename Vec> static Vec pow     (Vec a, Vec b)     { return a.performBinaryOp (b, [] (auto x, auto y) { return intrinsics::pow (x, y); }); }
            template <typename Vec> static Vec exp     (Vec a)            { return a.performUnaryOp ([] (auto x) { return intrinsics::exp (x); }); }

            template <typename Vec> static Vec rightShiftUnsigned (Vec a, Vec b) { return a.performBinaryOp (b, [] (auto x, auto y) { return intrinsics::rightShiftUnsigned (x, y); }); }
        };
    };

    static constexpr float  _inf32  =  std::numeric_limits<float>::infinity();
    static constexpr double _inf64  =  std::numeric_limits<double>::infinity();
    static constexpr float  _ninf32 = -std::numeric_limits<float>::infinity();
    static constexpr double _ninf64 = -std::numeric_limits<double>::infinity();
    static constexpr float  _nan32  =  std::numeric_limits<float>::quiet_NaN();
    static constexpr double _nan64  =  std::numeric_limits<double>::quiet_NaN();

    //==============================================================================
    #if __clang__
     #pragma clang diagnostic pop
    #elif __GNUC__
     #pragma GCC diagnostic pop
    #else
     #pragma warning (pop)
    #endif
};


