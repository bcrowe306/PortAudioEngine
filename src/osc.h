
#include <cstdint>
#include <cmath>
#include <cassert>
#include <string>
#include <cstring>
#include <array>

//==============================================================================
/// Auto-generated C++ class for the 'Untitled' processor
///

#if ! (defined (__cplusplus) && (__cplusplus >= 201703L))
 #error "This code requires that your compiler is set to use C++17 or later!"
#endif

struct OSC
{
    OSC() = default;
    ~OSC() = default;

    static constexpr std::string_view name = "Untitled";

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
    static constexpr uint32_t numInputEndpoints  = 1;
    static constexpr uint32_t numOutputEndpoints = 1;

    static constexpr uint32_t maxFramesPerBlock  = 512;
    static constexpr uint32_t eventBufferSize    = 32;
    static constexpr uint32_t maxOutputEventSize = 0;
    static constexpr double   latency            = 0.000000;

    enum class EndpointHandles
    {
        out       = 2,
        frequency = 1
    };

    static constexpr uint32_t getEndpointHandleForName (std::string_view endpointName)
    {
        if (endpointName == "out")        return static_cast<uint32_t> (EndpointHandles::out);
        if (endpointName == "frequency")  return static_cast<uint32_t> (EndpointHandles::frequency);
        return 0;
    }

    static constexpr EndpointInfo inputEndpoints[] =
    {
        { 1,  "frequency",  EndpointType::value }
    };

    static constexpr EndpointInfo outputEndpoints[] =
    {
        { 2,  "out",  EndpointType::stream }
    };

    //==============================================================================
    static constexpr uint32_t numAudioInputChannels  = 0;
    static constexpr uint32_t numAudioOutputChannels = 1;

    static constexpr std::array outputAudioStreams
    {
        outputEndpoints[0]
    };

    static constexpr std::array<EndpointInfo, 0> outputEvents {};

    static constexpr std::array<EndpointInfo, 0> outputMIDIEvents {};

    static constexpr std::array<EndpointInfo, 0> inputAudioStreams {};

    static constexpr std::array<EndpointInfo, 0> inputEvents {};

    static constexpr std::array<EndpointInfo, 0> inputMIDIEvents {};

    static constexpr std::array inputParameters
    {
        inputEndpoints[0]
    };

    static constexpr const char* programDetailsJSON =
            "{\n"
            "  \"mainProcessor\": \"Untitled\",\n"
            "  \"inputs\": [\n"
            "    {\n"
            "      \"endpointID\": \"frequency\",\n"
            "      \"endpointType\": \"value\",\n"
            "      \"dataType\": {\n"
            "        \"type\": \"float32\"\n"
            "      },\n"
            "      \"annotation\": {\n"
            "        \"name\": \"Frequency\",\n"
            "        \"min\": 5,\n"
            "        \"max\": 1000,\n"
            "        \"init\": 440\n"
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
    struct _Untitled_State
    {
        float phase = {};
        int32_t _sessionID = {};
        double _frequency = {};
        float _v_frequency = {};
        int32_t _resumeIndex = {};
    };

    struct Untitled_value_frequency
    {
        float value = {};
        float increment = {};
        int32_t frames = {};
    };

    struct Untitled_State
    {
        int32_t _activeRamps = {};
        int32_t _currentFrame = {};
        _Untitled_State _state;
        Untitled_value_frequency _v_frequency;
    };

    struct Untitled_IO
    {
        Array<float, 512> out;
    };

    struct _Untitled_IO
    {
        float out = {};
    };

    using std_intrinsics_T = float;
    using std_intrinsics_T_0 = float;
    using std_intrinsics_T_1 = float;

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
        _advance (cmajState, cmajIO, frames);
    }

    void copyOutputValue (EndpointHandle endpointHandle, void* dest)
    {
        (void) endpointHandle; (void) dest;

        assert (false);
    }

    void copyOutputFrames (EndpointHandle endpointHandle, void* dest, uint32_t numFramesToCopy)
    {
        if (endpointHandle == 2) { std::memcpy (reinterpret_cast<char*> (dest), std::addressof (cmajIO.out), 4 * numFramesToCopy); std::memset (reinterpret_cast<char*> (std::addressof (cmajIO.out)), 0, 4 * numFramesToCopy); return; }
        assert (false);
    }

    uint32_t getNumOutputEvents (EndpointHandle endpointHandle)
    {
        (void) endpointHandle;

        assert (false); return {};
    }

    void resetOutputEventCount (EndpointHandle endpointHandle)
    {
        (void) endpointHandle;
    }

    uint32_t getOutputEventType (EndpointHandle endpointHandle, uint32_t index)
    {
        (void) endpointHandle; (void) index;

        assert (false); return {};
    }

    static uint32_t getOutputEventDataSize (EndpointHandle endpointHandle, uint32_t typeIndex)
    {
        (void) endpointHandle; (void) typeIndex;

        assert (false); return 0;
    }

    uint32_t readOutputEvent (EndpointHandle endpointHandle, uint32_t index, unsigned char* dest)
    {

        (void) endpointHandle; (void) index; (void) dest;

        assert (false);
        return {};
    }

    void addEvent (EndpointHandle endpointHandle, uint32_t typeIndex, const unsigned char* eventData)
    {
        (void) endpointHandle; (void) typeIndex; (void) eventData;
    }

    void setValue (EndpointHandle endpointHandle, const void* value, int32_t frames)
    {
        if (endpointHandle == 1) return _setValue_frequency (cmajState, *(float*) value, frames);
    }

    void setInputFrames (EndpointHandle endpointHandle, const void* frameData, uint32_t numFrames, uint32_t numTrailingFramesToClear)
    {
        (void) endpointHandle; (void) frameData; (void) numFrames; (void) numTrailingFramesToClear;
    }

    //==============================================================================
    // Rendering state values
    int32_t initSessionID;
    double initFrequency;
    Untitled_State cmajState = {};
    Untitled_IO cmajIO = {};

    //==============================================================================
    void _setValue_frequency (Untitled_State& _state, float& value, int32_t frames) noexcept
    {
        if (frames == int32_t {0})
        {
            frames = int32_t {1};
        }
        if (_state._v_frequency.frames == int32_t {0})
        {
            ++_state._activeRamps;
        }
        _state._v_frequency.increment = ((value - _state._v_frequency.value) / static_cast<float> (frames));
        _state._v_frequency.frames = frames;
    }

    void _initialise (Untitled_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        _Untitled___initialise (_state._state, processorID, sessionID, frequency);
    }

    void _Untitled___initialise (_Untitled_State& _state, int32_t& processorID, int32_t sessionID, double frequency) noexcept
    {
        g__sessionID = sessionID;
        g__frequency = frequency;
    }

    void _advance (Untitled_State& _state, Untitled_IO& _io, int32_t _frames) noexcept
    {
        _Untitled_IO  ioCopy;

        for (;;)
        {
            if (_state._currentFrame == _frames)
            {
                break;
            }
            if (_state._activeRamps != int32_t {0})
            {
                Untitled___updateRamps (_state);
                _state._state._v_frequency = _state._v_frequency.value;
            }
            ioCopy = _Untitled_IO {};
            _Untitled__main (_state._state, ioCopy);
            _io.out[_state._currentFrame] = ioCopy.out;
            ++_state._currentFrame;
        }
        _state._currentFrame = int32_t {0};
    }

    void Untitled___updateRamps (Untitled_State& _state) noexcept
    {
        if (_state._v_frequency.frames != int32_t {0})
        {
            _state._v_frequency.value = (_state._v_frequency.value + _state._v_frequency.increment);
            --_state._v_frequency.frames;
            if (_state._v_frequency.frames == int32_t {0})
            {
                --_state._activeRamps;
            }
        }
    }

    void _Untitled__main (_Untitled_State& _state, _Untitled_IO& _io) noexcept
    {
        float  phaseDelta;

        for (;;)
        {
            _io.out = (_io.out + (g_volume * intrinsics::sin (_state.phase)));
            phaseDelta = static_cast<float> ((static_cast<double> (_state._v_frequency) * (1.0 / (1.0 * g__frequency))) * 6.283185307179586);
            _state.phase = intrinsics::addModulo2Pi (_state.phase, phaseDelta);
            return;
        }
    }

    float std__intrinsics__sin (float n) noexcept
    {
        {
            return 0.0f;
        }
    }

    float std__intrinsics__addModulo2Pi (float startValue, float valueToAdd) noexcept
    {
        float  v;

        v = startValue + valueToAdd;
        if (v < static_cast<float> (int32_t {0}))
        {
            return intrinsics::fmod (v, 6.2831855f) + 6.2831855f;
        }
        if (v < 6.2831855f)
        {
            return v;
        }
        if (v < 12.566371f)
        {
            return v - 6.2831855f;
        }
        return intrinsics::fmod (v, 6.2831855f);
    }

    float std__intrinsics__fmod (float x, float y) noexcept
    {
        {
            return x - (y * static_cast<float> (static_cast<int64_t> (x / y)));
        }
    }

    //==============================================================================
    const char* getStringForHandle (uint32_t handle, size_t& stringLength)
    {
        (void) handle; (void) stringLength;
        return "";
    }

    //==============================================================================
    int32_t g__sessionID {};
    double g__frequency {};
    static constexpr float g_volume { 0.15f };

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

