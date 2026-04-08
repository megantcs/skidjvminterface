#include "../includes/skidjvminterface.h"
#include <string.h>

static PHotspotContext ActivityContext = NULL;
static PVOID jvm = NULL;

static VMStructEntry* NextLinkEntry = NULL;
static VMStructEntry* NameEntry = NULL;
static VMStructEntry* JavaMirrorEntry = NULL;
static VMStructEntry* KlassesEntry = NULL;
static VMStructEntry* NextLoaderEntry = NULL;
static VMStructEntry* FieldinfoStreamEntry = NULL;
static VMStructEntry* ConstantsEntry = NULL;
static VMStructEntry* ConstantsSizeEntry = NULL;
static VMStructEntry* NullCheckEntry = NULL;
static VMStructEntry* SuperEntry = NULL;
static VMStructEntry* BaseEntry = NULL;
static VMStructEntry* ShiftEntry = NULL;
static VMStructEntry* FieldCounterEntry = NULL;

static VMStructEntry* KlassPointerBase = NULL;
static VMStructEntry* KlassPointerShift = NULL;



static void* GetPointer(void* pointer)
{
    void* buffer = 0;
    ApiReadmem(&buffer, pointer, sizeof(void*));
    return buffer;
}

static PVMStructEntry FindStructure(PCHAR typeName, PCHAR fieldName) {
    return ApiFindStructure(&ActivityContext->VMStructs, typeName, fieldName);
}

static VMStructEntry* Compressed17OopBaseAddres()
{
    return FindStructure("CompressedOops", "_narrow_oop._base");
}

static VMStructEntry* Compressed17OopShiftAddres()
{
    return FindStructure("CompressedOops", "_narrow_oop._shift");
}

static VMStructEntry* Compressed17GetNullCheck()
{
    return FindStructure("CompressedOops", "_narrow_oop._use_implicit_null_checks");
}

static VMStructEntry* CompressedPointer17Base()
{
    return FindStructure("CompressedKlassPointers", "_narrow_klass._base");
}

static VMStructEntry* CompressedPointer17Shift()
{
    return FindStructure("CompressedKlassPointers", "_narrow_klass._shift");
}

static VMStructEntry* ClassLoaderDataGraph17Head() { return FindStructure("ClassLoaderDataGraph", "_head"); }

static void* ClassLoaderDataGraph17GetClassLoader()
{
    VMStructEntry* headEntry = ClassLoaderDataGraph17Head();
    return GetPointer(headEntry->address);
}

static VMStructEntry* Next() { return FindStructure("ClassLoaderData", "_next"); }
static VMStructEntry* Klasses() { return FindStructure("ClassLoaderData", "_klasses"); }

static void* ClassLoaderData17GetNext() {
    VMStructEntry* entry = Next();
    return entry ? GetPointer(PTRMATH(ClassLoaderDataGraph17GetClassLoader() + entry->offset)) : (void*)0;
}

static void* ClassLoaderData17GetKlasses() {
    VMStructEntry* entry = Klasses();
    return entry ? GetPointer(PTRMATH(ClassLoaderDataGraph17GetClassLoader() + entry->offset)) : (void*)0;
}

static uintptr_t GetCompressedKlassPointersBase()
{
    uintptr_t base = 0;
    VMStructEntry* narrowKlassBase = FindStructure("CompressedKlassPointers", "_narrow_klass._base");
    if (narrowKlassBase)
    {
        ApiReadmem(&base, PTRMATH(jvm + (uintptr_t)narrowKlassBase->address), sizeof(uintptr_t));
    }
    return base;
}

static int GetCompressedKlassPointersOopShift()
{
    int shift = 0;
    VMStructEntry* narrowKlassShift = FindStructure("CompressedKlassPointers", "_narrow_klass._shift");
    if (narrowKlassShift)
    {
        ApiReadmem(&shift, PTRMATH(jvm + (uintptr_t)narrowKlassShift->address), sizeof(int));
    }
    return shift;
}


static uintptr_t GetCompressedOopBase()
{
    uintptr_t base = 0;
    VMStructEntry* narrowOopBase = FindStructure("CompressedOops", "_narrow_oop._base");
    if (narrowOopBase)
    {
        ApiReadmem(&base, PTRMATH(jvm + (uintptr_t)narrowOopBase->address), sizeof(uintptr_t));
    }
    return base;
}

static int GetCompressedOopShift()
{
    int shift = 0;
    VMStructEntry* narrowOopShift = FindStructure("CompressedOops", "_narrow_oop._shift");
    if (narrowOopShift)
    {
        ApiReadmem(&shift, PTRMATH(jvm + (uintptr_t)narrowOopShift->address), sizeof(int));
    }
    return shift;
};

static char GetCompressedNullChecks()
{
    char null_checks = 0;

    VMStructEntry* narrowOopNullChecks = FindStructure("CompressedOops", "_narrow_oop._use_implicit_null_checks");
    if (narrowOopNullChecks)
    {
        ApiReadmem(&null_checks, PTRMATH(jvm + (uintptr_t)narrowOopNullChecks->address), sizeof(null_checks));
    }
    return null_checks;
}

static VMStructEntry* ConstantPool17Length()
{
    return FindStructure("ConstantPool", "_length");
}

static VMStructEntry* KlassNextLink()
{
    return FindStructure("Klass", "_next_link");
}

static VMStructEntry* KlassName()
{
    return FindStructure("Klass", "_name");
}

static VMStructEntry* KlassJavaMirror()
{
    return FindStructure("Klass", "_java_mirror");
}

static VMStructEntry* KlassSuper()
{
    return FindStructure("Klass", "_super");
}

static VMStructEntry* InstanceKlassFieldinfoStream()
{
    VMStructEntry* entry = FindStructure("InstanceKlass", "_fields");
    if (!entry)
    {
        entry = FindStructure("InstanceKlass", "_fieldinfo_stream");
    }
    return entry;
}

static VMStructEntry* InstanceKlassConstants()
{
    return FindStructure("InstanceKlass", "_constants");
}

static VMStructEntry* InstanceKlassMethods()
{
    return FindStructure("InstanceKlass", "_methods");
}

static VMStructEntry* InstanceKlassJavaFieldsCount()
{
    return FindStructure("InstanceKlass", "_java_fields_count");
}

static void InitEntries(void)
{
    static int initialized = 0;
    if (initialized) return;

    NextLinkEntry = KlassNextLink();
    NameEntry = KlassName();
    JavaMirrorEntry = KlassJavaMirror();
    KlassesEntry = Klasses();
    NextLoaderEntry = Next();
    FieldinfoStreamEntry = InstanceKlassFieldinfoStream();
    ConstantsEntry = InstanceKlassConstants();
    ConstantsSizeEntry = ConstantPool17Length();
    NullCheckEntry = Compressed17GetNullCheck();
    SuperEntry = KlassSuper();
    ShiftEntry = Compressed17OopShiftAddres();
    BaseEntry = Compressed17OopBaseAddres();
    KlassPointerBase = CompressedPointer17Base();
    KlassPointerShift = CompressedPointer17Shift();
    FieldCounterEntry = InstanceKlassJavaFieldsCount();

    initialized = 1;
}

static inline uint16_t FieldInfo17NameIndex(const FieldInfo17* f) {
    return f->_shorts[NameIndexOffset];
}

static inline uint16_t FieldInfo17SignatureIndex(const FieldInfo17* f) {
    return f->_shorts[SignatureIndexOffset];
}

static inline int FieldInfo17Offset(const FieldInfo17* f) {
    return (((int)(((unsigned int)f->_shorts[HighPackedOffset] << 16) | (unsigned int)f->_shorts[LowPackedOffset])) >> 2);
}

static inline int AccessFlagsIsStatic(const AccessFlags* af) {
    return (af->_flags & 0x0008) != 0;
}

static inline void SymbolRead(Symbol* sym, void* address) {
    ApiReadmem(sym, address, sizeof(Symbol));
}

static inline void SymbolGetString(const Symbol* sym, char* buffer, size_t size) {
    size_t len = sym->_length < size - 1 ? sym->_length : size - 1;
    memcpy(buffer, sym->_body, len);
    buffer[len] = '\0';
}

static uint32_t EncodeOop(void* oop) {
    uintptr_t base = GetCompressedOopBase();
    int shift = GetCompressedOopShift();
    uintptr_t oopAddr = (uintptr_t)oop;
    return (uint32_t)((oopAddr - base) >> shift);
}

static void* DecodeOop(uint32_t compressedOop) {
    uintptr_t base = GetCompressedOopBase();
    int shift = GetCompressedOopShift();
    return (void*)(((uintptr_t)compressedOop << shift) + base);
}

static void* DecodeKlass(jclass klass)
{
    uintptr_t value = (uintptr_t)klass;

    if (NullCheckEntry)
    {
        char flag;
        void* base;
        int shift;
        flag = GetCompressedNullChecks();
        base = (void*)GetCompressedKlassPointersBase();
        shift = GetCompressedKlassPointersOopShift();

        assert(flag || base || shift);

        if (flag)
        {
            value = (value << shift) + (uintptr_t)base;
        }
    }
    return (jclass)klass;
};


static jobject DecodeOopJobject(jobject oop) {
    uintptr_t base = 0;
    int shift = 0;
    uintptr_t oopValue = (uintptr_t)oop;

    if (ShiftEntry && BaseEntry) {

        ApiReadmem(&base, BaseEntry->address, sizeof(base));
        ApiReadmem(&shift, ShiftEntry->address, sizeof(shift));
    }

    if (shift > 0 && (oopValue & 0xFFFFFFFF00000000) == 0) {
        uintptr_t decoded = (oopValue << shift) + base;
        return (jobject)decoded;
    }
    return oop;
}

static jclass FindClass(PCHAR name) {
    if (!name) return NULL;

    InitEntries();

    void* currentLoader = ClassLoaderDataGraph17GetClassLoader();
    if (!currentLoader) return NULL;

    while (currentLoader) {
        void* currentKlass = GetPointer(PTRMATH((uintptr_t)currentLoader + (uintptr_t)KlassesEntry->offset));

        while (currentKlass) {
            if (NameEntry) {
                void* symbolPtr = GetPointer(PTRMATH((uintptr_t)currentKlass + (uintptr_t)NameEntry->offset));
                if (symbolPtr) {
                    Symbol symbol;
                    SymbolRead(&symbol, symbolPtr);
                    char className[256] = { 0 };
                    SymbolGetString(&symbol, className, sizeof(className));

                    if (strcmp(className, name) == 0) {
                        return (jclass)currentKlass;
                    }
                }
            }
            currentKlass = GetPointer(PTRMATH((uintptr_t)currentKlass + (uintptr_t)NextLinkEntry->offset));
        }
        currentLoader = GetPointer(PTRMATH((uintptr_t)currentLoader + (uintptr_t)NextLoaderEntry->offset));
    }

    return NULL;
}

static uint32_t read_string_leb2(const uint8_t* data, size_t* pos)
{
    uint32_t sum = data[(*pos)++] - 1;
    if (sum < 191) return sum;

    int shift = 6;
    while (1) {
        uint8_t b = data[(*pos)++];
        sum += (b - 1) << shift;
        if (b < (1 + 191)) break;
        shift += 6;
    }
    return sum;
}

static void read_field_info20(FieldInfo20* out, const uint8_t* buffer, size_t* pos)
{
    out->_name_index = (uint16_t)read_string_leb2(buffer, pos);
    out->_signature_index = (uint16_t)read_string_leb2(buffer, pos);
    out->_offset = read_string_leb2(buffer, pos);
    uint16_t access_flags_value = (uint16_t)read_string_leb2(buffer, pos);
    access_flags_init(&out->_access_flags, access_flags_value);

    uint32_t field_flags_value = read_string_leb2(buffer, pos);
    field_flags_init(&out->_field_flags, field_flags_value);

    if (field_flags_is_initialized(&out->_field_flags)) {
        out->_initializer_index = (uint16_t)read_string_leb2(buffer, pos);
    }
    if (field_flags_is_generic(&out->_field_flags)) {
        out->_generic_signature_index = (uint16_t)read_string_leb2(buffer, pos);
    }
    if (field_flags_is_contended(&out->_field_flags)) {
        out->_contention_group = (uint16_t)read_string_leb2(buffer, pos);
    }
}


static jfieldID FindField(jclass clazz, PCHAR fieldName, PCHAR signature) {
    if (!clazz || !fieldName) {
        return 0;
    }

    if (!ConstantsEntry || !ConstantsSizeEntry || !FieldinfoStreamEntry) {
        return 0;
    }

    void* constants;
    if (!ApiReadmem(&constants, PTRMATH((uintptr_t)clazz + ConstantsEntry->offset), sizeof(constants))) {
        return 0;
    }

    size_t constPoolSize = ConstantsSizeEntry->offset + sizeof(int) + sizeof(void*);
    uintptr_t base = (uintptr_t)constants + constPoolSize;

    void* fieldInfo = GetPointer(PTRMATH((uintptr_t)clazz + FieldinfoStreamEntry->offset));
    if (!fieldInfo) {
        return 0;
    }

    int length = 0;
    if (!ApiReadmem(&length, fieldInfo, sizeof(length))) {
        return 0;
    }

    if (length <= 0) {
        return 0;
    }

    uint8_t* buffer = (uint8_t*)malloc(length);
    if (!buffer) {
        return 0;
    }

    if (!ApiReadmem(buffer, (void*)((uintptr_t)fieldInfo + sizeof(int)), length)) {
        free(buffer);
        return 0;
    }

    size_t pos = 0;

    uint32_t java_count = read_string_leb2(buffer, &pos);
    uint32_t injected_count = read_string_leb2(buffer, &pos);
    uint32_t total = java_count + injected_count;

    jfieldID result = 0;
    for (uint32_t i = 0; i < total; ++i) {
        FieldInfo20 field;
        field_info20_init_default(&field);
        field._index = i;

        read_field_info20(&field, buffer, &pos);

        uintptr_t nameAddr = base + (field._name_index * sizeof(intptr_t));
        uintptr_t sigAddr = base + (field._signature_index * sizeof(intptr_t));

        void* namePtr = GetPointer((void*)nameAddr);
        void* sigPtr = GetPointer((void*)sigAddr);

        if (namePtr && sigPtr) {
            Symbol nameSymbol, sigSymbol;
            SymbolRead(&nameSymbol, namePtr);
            SymbolRead(&sigSymbol, sigPtr);

            char currentName[256] = { 0 };
            char currentSig[256] = { 0 };
            SymbolGetString(&nameSymbol, currentName, sizeof(currentName));
            SymbolGetString(&sigSymbol, currentSig, sizeof(currentSig));

            if (strcmp(currentName, fieldName) == 0) {
                if (signature == NULL || strcmp(currentSig, signature) == 0) {
                    result = (jfieldID)(uintptr_t)field._offset;
                    break;
                }
            }
        }
    }

    free(buffer);
    return result;
}

static jint GetArrayLen(jobject oop) {
    jint length = 0;
    int offset = (NullCheckEntry && NullCheckEntry->address) ? 0xC : 0x10;
    ApiReadmem(&length, PTRMATH((uintptr_t)oop + offset), sizeof(length));
    return length;
}

static void GetObjectArrayElement(jobject oop, jobject* array, int start, int end) {
    int base = (NullCheckEntry && NullCheckEntry->address) ? 0x10 : 0x18;
    jint length = GetArrayLen(oop);
    end += start;

    for (int i = start; i < end && i < length; i++) {
        uint32_t val = 0;
        ApiReadmem(&val, PTRMATH((uintptr_t)oop + base + i * sizeof(unsigned int)), sizeof(uint32_t));
        array[i - start] = DecodeOopJobject((jobject)(uintptr_t)val);
    }
}

static jobject GetObjectField(jobject obj, jfieldID fieldID) {
    uint32_t val = 0;
    ApiReadmem(&val, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(uint32_t));
    return DecodeOopJobject((jobject)(uintptr_t)val);
}

static jobject GetFieldObject(jobject obj, jfieldID fieldID) {
    return GetObjectField(obj, fieldID);
}

static jint GetFieldInt(jobject obj, jfieldID fieldID) {
    jint value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jint));
    return value;
}

static jfloat GetFieldFloat(jobject obj, jfieldID fieldID) {
    jfloat value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jfloat));
    return value;
}

static jdouble GetFieldDouble(jobject obj, jfieldID fieldID) {
    jdouble value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jdouble));
    return value;
}

static jboolean GetFieldBoolean(jobject obj, jfieldID fieldID) {
    jboolean value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jboolean));
    return value;
}

static jbyte GetFieldByte(jobject obj, jfieldID fieldID) {
    jbyte value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jbyte));
    return value;
}

static jchar GetFieldChar(jobject obj, jfieldID fieldID) {
    jchar value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jchar));
    return value;
}

static jshort GetFieldShort(jobject obj, jfieldID fieldID) {
    jshort value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jshort));
    return value;
}

static jlong GetFieldLong(jobject obj, jfieldID fieldID) {
    jlong value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jlong));
    return value;
}

static jobject GetStaticFieldObject(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return NULL;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return NULL;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return NULL;

    uint32_t compressedValue;
    if (!ApiReadmem(&compressedValue, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(uint32_t))) return NULL;

    return DecodeOopJobject((jobject)(uintptr_t)compressedValue);
}

static jint GetStaticFieldInt(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jint value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jint));
    return value;
}

static jfloat GetStaticFieldFloat(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jfloat value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jfloat));
    return value;
}

static jdouble GetStaticFieldDouble(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jdouble value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jdouble));
    return value;
}

static jboolean GetStaticFieldBoolean(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jboolean value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jboolean));
    return value;
}

static jbyte GetStaticFieldByte(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jbyte value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jbyte));
    return value;
}

static jchar GetStaticFieldChar(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jchar value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jchar));
    return value;
}

static jshort GetStaticFieldShort(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jshort value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jshort));
    return value;
}

static jlong GetStaticFieldLong(jclass clazz, jfieldID fieldID) {
    if (!clazz || !fieldID) return 0;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return 0;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return 0;

    jlong value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jlong));
    return value;
}

static void SetFieldObject(jobject obj, jfieldID fieldID, jobject value) {
    if (!obj || !fieldID) return;
    uint32_t compressed = EncodeOop((void*)value);
    ApiWritemem(&compressed, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(uint32_t));
}

static void SetFieldInt(jobject obj, jfieldID fieldID, jint value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jint));
}

static void SetFieldFloat(jobject obj, jfieldID fieldID, jfloat value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jfloat));
}

static void SetFieldDouble(jobject obj, jfieldID fieldID, jdouble value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jdouble));
}

static void SetFieldBoolean(jobject obj, jfieldID fieldID, jboolean value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jboolean));
}

static void SetFieldByte(jobject obj, jfieldID fieldID, jbyte value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jbyte));
}

static void SetFieldChar(jobject obj, jfieldID fieldID, jchar value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jchar));
}

static void SetFieldShort(jobject obj, jfieldID fieldID, jshort value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jshort));
}

static void SetFieldLong(jobject obj, jfieldID fieldID, jlong value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jlong));
}

static void SetStaticFieldObject(jclass clazz, jfieldID fieldID, jobject value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    uint32_t compressed = EncodeOop((void*)value);
    ApiWritemem(&compressed, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(uint32_t));
}

static void SetStaticFieldInt(jclass clazz, jfieldID fieldID, jint value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jint));
}

static void SetStaticFieldFloat(jclass clazz, jfieldID fieldID, jfloat value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jfloat));
}

static void SetStaticFieldDouble(jclass clazz, jfieldID fieldID, jdouble value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jdouble));
}

static void SetStaticFieldBoolean(jclass clazz, jfieldID fieldID, jboolean value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jboolean));
}

static void SetStaticFieldByte(jclass clazz, jfieldID fieldID, jbyte value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jbyte));
}

static void SetStaticFieldChar(jclass clazz, jfieldID fieldID, jchar value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jchar));
}

static void SetStaticFieldShort(jclass clazz, jfieldID fieldID, jshort value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jshort));
}

static void SetStaticFieldLong(jclass clazz, jfieldID fieldID, jlong value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jlong));
}
static jclass GetObjectClass(jobject object)
{
    if (!object) return 0;

    static const int KLASS_OFFSET = 0x8;

    boolean compressed = 0;
    if (NullCheckEntry) {
        ApiReadmem(&compressed, NullCheckEntry->address, sizeof(boolean));
    }

    if (compressed) {
        uint32_t compressedKlass;
        if (!ApiReadmem(&compressedKlass, PTRMATH((uintptr_t)object + KLASS_OFFSET), sizeof(uint32_t))) {
            return 0;
        }

        uintptr_t base = 0;
        int shift = 0;

        if (KlassPointerBase) {
            ApiReadmem(&base, KlassPointerBase->address, sizeof(uintptr_t));
        }

        if (KlassPointerShift) {
            ApiReadmem(&shift, KlassPointerShift->address, sizeof(int));
        }

        uintptr_t klassAddr = ((uintptr_t)compressedKlass << shift) + base;
        return (jclass)klassAddr;
    }
    else {
        jclass klass;
        if (!ApiReadmem(&klass, PTRMATH((uintptr_t)object + KLASS_OFFSET), sizeof(jclass))) {
            return 0;
        }
        return klass;
    }
}

static jboolean IsInstanceOf(jobject obj, jclass clazz) {
    if (!obj || !clazz) return 0;

    jclass objClass = GetObjectClass(obj);
    if (!objClass) return 0;

    if (objClass == clazz) return 1;

    jclass current = objClass;
    int maxDepth = 50;

    while (current && maxDepth-- > 0) {
        jclass superClass = NULL;
        if (!ApiReadmem(&superClass, PTRMATH((uintptr_t)current + (uintptr_t)SuperEntry->offset), sizeof(jclass))) {
            break;
        }
        if (!superClass) break;
        if (superClass == clazz) return 1;
        if (superClass == current) break;
        current = superClass;
    }

    return 0;
}

static void MonitorEnter(jobject obj) {
    (void)obj;
}

static void MonitorExit(jobject obj) {
    (void)obj;
}

static void SetObjectField(jobject obj, jfieldID fieldID, jobject value) {
    SetFieldObject(obj, fieldID, value);
}

static jboolean IsSameObject(jobject obj1, jobject obj2) {
    return obj1 == obj2 ? 1 : 0;
}

static jclass GetSuperclass(jclass clazz) {
    if (!clazz) return NULL;
    jclass superClass = NULL;
    ApiReadmem(&superClass, PTRMATH((uintptr_t)clazz + (uintptr_t)SuperEntry->offset), sizeof(jclass));
    return superClass;
}

static jboolean IsAssignableFrom(jclass sub, jclass super) {
    if (!sub || !super) return 0;
    if (sub == super) return 1;

    jclass current = sub;
    int maxDepth = 50;

    while (current && maxDepth-- > 0) {
        jclass superClass = NULL;
        if (!ApiReadmem(&superClass, PTRMATH((uintptr_t)current + (uintptr_t)SuperEntry->offset), sizeof(jclass))) {
            break;
        }
        if (!superClass) break;
        if (superClass == super) return 1;
        if (superClass == current) break;
        current = superClass;
    }

    return 0;
}

static jobject AllocObject(jclass clazz) {
    (void)clazz;
    return NULL;
}

SJStatus ApiNewJvmInterfaceFor21J(In_ PHotspotContext Context, Out_ PIJVMINTERFACE* Interface)
{
    ActivityContext = Context;
    jvm = Context->Proc->hJvmDll;

    InitEntries();

    PIJVMINTERFACE OutInterface = malloc(sizeof(IJVMINTERFACE));
    if (!OutInterface) {
        return SJUnhandledError;
    }

    OutInterface->findClass = FindClass;
    OutInterface->findField = FindField;
    OutInterface->getArrayLen = GetArrayLen;
    OutInterface->getObjectField = GetObjectField;
    OutInterface->isInstanceOf = IsInstanceOf;
    OutInterface->getObjectClass = GetObjectClass;

    OutInterface->getFieldObject = GetFieldObject;
    OutInterface->getFieldInt = GetFieldInt;
    OutInterface->getFieldFloat = GetFieldFloat;
    OutInterface->getFieldDouble = GetFieldDouble;
    OutInterface->getFieldBoolean = GetFieldBoolean;
    OutInterface->getFieldByte = GetFieldByte;
    OutInterface->getFieldChar = GetFieldChar;
    OutInterface->getFieldShort = GetFieldShort;
    OutInterface->getFieldLong = GetFieldLong;

    OutInterface->getStaticFieldObject = GetStaticFieldObject;
    OutInterface->getStaticFieldInt = GetStaticFieldInt;
    OutInterface->getStaticFieldFloat = GetStaticFieldFloat;
    OutInterface->getStaticFieldDouble = GetStaticFieldDouble;
    OutInterface->getStaticFieldBoolean = GetStaticFieldBoolean;
    OutInterface->getStaticFieldByte = GetStaticFieldByte;
    OutInterface->getStaticFieldChar = GetStaticFieldChar;
    OutInterface->getStaticFieldShort = GetStaticFieldShort;
    OutInterface->getStaticFieldLong = GetStaticFieldLong;

    OutInterface->setFieldObject = SetFieldObject;
    OutInterface->setFieldInt = SetFieldInt;
    OutInterface->setFieldFloat = SetFieldFloat;
    OutInterface->setFieldDouble = SetFieldDouble;
    OutInterface->setFieldBoolean = SetFieldBoolean;
    OutInterface->setFieldByte = SetFieldByte;
    OutInterface->setFieldChar = SetFieldChar;
    OutInterface->setFieldShort = SetFieldShort;
    OutInterface->setFieldLong = SetFieldLong;

    OutInterface->setStaticFieldObject = SetStaticFieldObject;
    OutInterface->setStaticFieldInt = SetStaticFieldInt;
    OutInterface->setStaticFieldFloat = SetStaticFieldFloat;
    OutInterface->setStaticFieldDouble = SetStaticFieldDouble;
    OutInterface->setStaticFieldBoolean = SetStaticFieldBoolean;
    OutInterface->setStaticFieldByte = SetStaticFieldByte;
    OutInterface->setStaticFieldChar = SetStaticFieldChar;
    OutInterface->setStaticFieldShort = SetStaticFieldShort;
    OutInterface->setStaticFieldLong = SetStaticFieldLong;

    OutInterface->setObjectField = SetObjectField;
    OutInterface->getObjectArrayElement = GetObjectArrayElement;
    OutInterface->monitorEnter = MonitorEnter;
    OutInterface->monitorExit = MonitorExit;
    OutInterface->isSameObject = IsSameObject;
    OutInterface->getSuperclass = GetSuperclass;
    OutInterface->isAssignableFrom = IsAssignableFrom;
    OutInterface->allocObject = AllocObject;

    *Interface = OutInterface;
    return SJSuccess;
}