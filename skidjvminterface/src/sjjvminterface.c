#include "../includes/skidjvminterface.h"
#include <string.h>

PHotspotContext ActivityContext = NULL;
PVOID jvm = NULL;

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

void* GetPointer(void* pointer)
{
    void* buffer = 0;
    ApiReadmem(&buffer, pointer, sizeof(void*));
    return buffer;
}

PVMStructEntry FindStructure(PCHAR typeName, PCHAR fieldName) {
    return ApiFindStructure(&ActivityContext->_vmStructs, typeName, fieldName);
}

VMStructEntry* Compressed17OopBaseAddres()
{
    return FindStructure("CompressedOops", "_narrow_oop._base");
}

VMStructEntry* Compressed17OopShiftAddres()
{
    return FindStructure("CompressedOops", "_narrow_oop._shift");
}

VMStructEntry* Compressed17GetNullCheck()
{
    return FindStructure("CompressedOops", "_narrow_oop._use_implicit_null_checks");
}

VMStructEntry* CompressedPointer17Base()
{
    return FindStructure("CompressedKlassPointers", "_narrow_klass._base");
}

VMStructEntry* CompressedPointer17Shift()
{
    return FindStructure("CompressedKlassPointers", "_narrow_klass._shift");
}

VMStructEntry* ClassLoaderDataGraph17Head() { return FindStructure("ClassLoaderDataGraph", "_head"); }

void* ClassLoaderDataGraph17GetClassLoader()
{
    VMStructEntry* headEntry = ClassLoaderDataGraph17Head();
    return GetPointer(headEntry->address);
}

VMStructEntry* Next() { return FindStructure("ClassLoaderData", "_next"); }
VMStructEntry* Klasses() { return FindStructure("ClassLoaderData", "_klasses"); }

void* ClassLoaderData17GetNext() {
    VMStructEntry* entry = Next();
    return entry ? GetPointer(PTRMATH(ClassLoaderDataGraph17GetClassLoader() + entry->offset)) : (void*)0;
}

void* ClassLoaderData17GetKlasses() {
    VMStructEntry* entry = Klasses();
    return entry ? GetPointer(PTRMATH(ClassLoaderDataGraph17GetClassLoader() + entry->offset)) : (void*)0;
}

uintptr_t GetCompressedOopBase()
{
    uintptr_t base = 0;
    VMStructEntry* narrowOopBase = FindStructure("CompressedOops", "_narrow_oop._base");
    if (narrowOopBase)
    {
        ApiReadmem(&base, PTRMATH(jvm + (uintptr_t)narrowOopBase->address), sizeof(uintptr_t));
    }
    return base;
}

int GetCompressedOopShift()
{
    int shift = 0;
    VMStructEntry* narrowOopShift = FindStructure("CompressedOops", "_narrow_oop._shift");
    if (narrowOopShift)
    {
        ApiReadmem(&shift, PTRMATH(jvm + (uintptr_t)narrowOopShift->address), sizeof(uintptr_t));
    }
    return shift;
}

VMStructEntry* ConstantPool17Length()
{
    return FindStructure("ConstantPool", "_length");
}

VMStructEntry* KlassNextLink()
{
    return FindStructure("Klass", "_next_link");
}

VMStructEntry* KlassName()
{
    return FindStructure("Klass", "_name");
}

VMStructEntry* KlassJavaMirror()
{
    return FindStructure("Klass", "_java_mirror");
}

VMStructEntry* KlassSuper()
{
    return FindStructure("Klass", "_super");
}

VMStructEntry* InstanceKlassFieldinfoStream()
{
    VMStructEntry* entry = FindStructure("InstanceKlass", "_fields");
    if (!entry)
    {
        entry = FindStructure("InstanceKlass", "_fieldinfo_stream");
    }
    return entry;
}

VMStructEntry* InstanceKlassConstants()
{
    return FindStructure("InstanceKlass", "_constants");
}

VMStructEntry* InstanceKlassMethods()
{
    return FindStructure("InstanceKlass", "_methods");
}

VMStructEntry* InstanceKlassJavaFieldsCount()
{
    return FindStructure("InstanceKlass", "_java_fields_count");
}

void InitEntries(void)
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
    ShiftEntry = CompressedPointer17Shift();
    BaseEntry = CompressedPointer17Base();
    FieldCounterEntry = InstanceKlassJavaFieldsCount();

    initialized = 1;
}

typedef struct {
    uint16_t _shorts[6];
} FieldInfo17;

enum FieldOffset {
    AccessFlagsOffset = 0,
    NameIndexOffset = 1,
    SignatureIndexOffset = 2,
    InitvalIndexOffset = 3,
    LowPackedOffset = 4,
    HighPackedOffset = 5,
    FieldSlots = 6
};

static inline uint16_t FieldInfo17NameIndex(const FieldInfo17* f) {
    return f->_shorts[NameIndexOffset];
}

static inline uint16_t FieldInfo17SignatureIndex(const FieldInfo17* f) {
    return f->_shorts[SignatureIndexOffset];
}

static inline int FieldInfo17Offset(const FieldInfo17* f) {
    return (((int)(((unsigned int)f->_shorts[HighPackedOffset] << 16) | (unsigned int)f->_shorts[LowPackedOffset])) >> 2);
}

typedef struct {
    uint16_t _flags;
} AccessFlags;

static inline int AccessFlagsIsStatic(const AccessFlags* af) {
    return (af->_flags & 0x0008) != 0;
}

typedef struct {
    uint32_t phar;
    uint16_t _length;
    char _body[255];
} Symbol;

static inline void SymbolRead(Symbol* sym, void* address) {
    ApiReadmem(sym, address, sizeof(Symbol));
}

static inline void SymbolGetString(const Symbol* sym, char* buffer, size_t size) {
    size_t len = sym->_length < size - 1 ? sym->_length : size - 1;
    memcpy(buffer, sym->_body, len);
    buffer[len] = '\0';
}

uint32_t EncodeOop(void* oop) {
    uintptr_t base = GetCompressedOopBase();
    int shift = GetCompressedOopShift();
    uintptr_t oopAddr = (uintptr_t)oop;
    return (uint32_t)((oopAddr - base) >> shift);
}

void* DecodeOop(uint32_t compressedOop) {
    uintptr_t base = GetCompressedOopBase();
    int shift = GetCompressedOopShift();
    return (void*)(((uintptr_t)compressedOop << shift) + base);
}

jobject DecodeOopJobject(jobject oop) {
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

jclass FindClass(PCHAR name) {
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

jfieldID FindField(jclass clazz, PCHAR fieldName, PCHAR signature) {
    if (!clazz || !fieldName) return 0;

    InitEntries();

    void* constants = NULL;
    void* fields = NULL;
    uint16_t fieldCount = 0;

    if (!ApiReadmem(&constants, PTRMATH((uintptr_t)clazz + (uintptr_t)ConstantsEntry->offset), sizeof(void*)) ||
        !ApiReadmem(&fields, PTRMATH((uintptr_t)clazz + (uintptr_t)FieldinfoStreamEntry->offset), sizeof(void*)) ||
        !ApiReadmem(&fieldCount, PTRMATH((uintptr_t)clazz + (uintptr_t)FieldCounterEntry->offset), sizeof(uint16_t))) {
        return 0;
    }

    fields = PTRMATH((uintptr_t)fields + sizeof(int));
    uintptr_t symbolBase = (uintptr_t)constants + (uintptr_t)ConstantsSizeEntry->offset + sizeof(int) + sizeof(void*);

    for (uint16_t i = 0; i < fieldCount; i++) {
        FieldInfo17 field;
        uintptr_t fieldAddr = (uintptr_t)fields + (i * 6 * sizeof(unsigned short));

        if (!ApiReadmem(&field, (void*)fieldAddr, sizeof(FieldInfo17))) continue;

        uintptr_t nameAddr = symbolBase + (FieldInfo17NameIndex(&field) * sizeof(void*));
        uintptr_t sigAddr = symbolBase + (FieldInfo17SignatureIndex(&field) * sizeof(void*));

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

            if (strcmp(currentName, fieldName) == 0 && (!signature || strcmp(currentSig, signature) == 0)) {
                return (jfieldID)(uintptr_t)FieldInfo17Offset(&field);
            }
        }
    }

    return 0;
}

jint GetArrayLen(jobject oop) {
    jint length = 0;
    int offset = (NullCheckEntry && NullCheckEntry->address) ? 0xC : 0x10;
    ApiReadmem(&length, PTRMATH((uintptr_t)oop + offset), sizeof(length));
    return length;
}

void GetObjectArrayElement(jobject oop, jobject* array, int start, int end) {
    int base = (NullCheckEntry && NullCheckEntry->address) ? 0x10 : 0x18;
    jint length = GetArrayLen(oop);
    end += start;

    for (int i = start; i < end && i < length; i++) {
        uint32_t val = 0;
        ApiReadmem(&val, PTRMATH((uintptr_t)oop + base + i * sizeof(unsigned int)), sizeof(uint32_t));
        array[i - start] = DecodeOopJobject((jobject)(uintptr_t)val);
    }
}

jobject GetObjectField(jobject obj, jfieldID fieldID) {
    uint32_t val = 0;
    ApiReadmem(&val, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(uint32_t));
    return DecodeOopJobject((jobject)(uintptr_t)val);
}

jobject GetFieldObject(jobject obj, jfieldID fieldID) {
    return GetObjectField(obj, fieldID);
}

jint GetFieldInt(jobject obj, jfieldID fieldID) {
    jint value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jint));
    return value;
}

jfloat GetFieldFloat(jobject obj, jfieldID fieldID) {
    jfloat value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jfloat));
    return value;
}

jdouble GetFieldDouble(jobject obj, jfieldID fieldID) {
    jdouble value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jdouble));
    return value;
}

jboolean GetFieldBoolean(jobject obj, jfieldID fieldID) {
    jboolean value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jboolean));
    return value;
}

jbyte GetFieldByte(jobject obj, jfieldID fieldID) {
    jbyte value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jbyte));
    return value;
}

jchar GetFieldChar(jobject obj, jfieldID fieldID) {
    jchar value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jchar));
    return value;
}

jshort GetFieldShort(jobject obj, jfieldID fieldID) {
    jshort value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jshort));
    return value;
}

jlong GetFieldLong(jobject obj, jfieldID fieldID) {
    jlong value = 0;
    ApiReadmem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jlong));
    return value;
}

jobject GetStaticFieldObject(jclass clazz, jfieldID fieldID) {
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

jint GetStaticFieldInt(jclass clazz, jfieldID fieldID) {
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

jfloat GetStaticFieldFloat(jclass clazz, jfieldID fieldID) {
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

jdouble GetStaticFieldDouble(jclass clazz, jfieldID fieldID) {
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

jboolean GetStaticFieldBoolean(jclass clazz, jfieldID fieldID) {
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

jbyte GetStaticFieldByte(jclass clazz, jfieldID fieldID) {
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

jchar GetStaticFieldChar(jclass clazz, jfieldID fieldID) {
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

jshort GetStaticFieldShort(jclass clazz, jfieldID fieldID) {
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

jlong GetStaticFieldLong(jclass clazz, jfieldID fieldID) {
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

void SetFieldObject(jobject obj, jfieldID fieldID, jobject value) {
    if (!obj || !fieldID) return;
    uint32_t compressed = EncodeOop((void*)value);
    ApiWritemem(&compressed, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(uint32_t));
}

void SetFieldInt(jobject obj, jfieldID fieldID, jint value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jint));
}

void SetFieldFloat(jobject obj, jfieldID fieldID, jfloat value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jfloat));
}

void SetFieldDouble(jobject obj, jfieldID fieldID, jdouble value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jdouble));
}

void SetFieldBoolean(jobject obj, jfieldID fieldID, jboolean value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jboolean));
}

void SetFieldByte(jobject obj, jfieldID fieldID, jbyte value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jbyte));
}

void SetFieldChar(jobject obj, jfieldID fieldID, jchar value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jchar));
}

void SetFieldShort(jobject obj, jfieldID fieldID, jshort value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jshort));
}

void SetFieldLong(jobject obj, jfieldID fieldID, jlong value) {
    if (!obj || !fieldID) return;
    ApiWritemem(&value, PTRMATH((uintptr_t)obj + (uintptr_t)fieldID), sizeof(jlong));
}

void SetStaticFieldObject(jclass clazz, jfieldID fieldID, jobject value) {
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

void SetStaticFieldInt(jclass clazz, jfieldID fieldID, jint value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jint));
}

void SetStaticFieldFloat(jclass clazz, jfieldID fieldID, jfloat value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jfloat));
}

void SetStaticFieldDouble(jclass clazz, jfieldID fieldID, jdouble value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jdouble));
}

void SetStaticFieldBoolean(jclass clazz, jfieldID fieldID, jboolean value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jboolean));
}

void SetStaticFieldByte(jclass clazz, jfieldID fieldID, jbyte value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jbyte));
}

void SetStaticFieldChar(jclass clazz, jfieldID fieldID, jchar value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jchar));
}

void SetStaticFieldShort(jclass clazz, jfieldID fieldID, jshort value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jshort));
}

void SetStaticFieldLong(jclass clazz, jfieldID fieldID, jlong value) {
    if (!clazz || !fieldID) return;

    jobject javaMirror;
    if (!ApiReadmem(&javaMirror, PTRMATH((uintptr_t)clazz + (uintptr_t)JavaMirrorEntry->offset), sizeof(jobject))) {
        return;
    }

    jobject classOOP = GetPointer((void*)javaMirror);
    if (!classOOP) return;

    ApiWritemem(&value, PTRMATH((uintptr_t)classOOP + (uintptr_t)fieldID), sizeof(jlong));
}

jclass GetObjectClass(jobject obj) {
    if (!obj) return NULL;

    int klassOffset = 0x8;
    int compressed = (NullCheckEntry && NullCheckEntry->address) ? 1 : 0;

    if (compressed) {
        uint32_t compressedKlass = 0;
        if (!ApiReadmem(&compressedKlass, PTRMATH((uintptr_t)obj + klassOffset), sizeof(uint32_t))) {
            return NULL;
        }

        uintptr_t base = 0;
        int shift = 0;
        if (BaseEntry) ApiReadmem(&base, BaseEntry->address, sizeof(base));
        if (ShiftEntry) ApiReadmem(&shift, ShiftEntry->address, sizeof(shift));

        uintptr_t klassAddr = ((uintptr_t)compressedKlass << shift) + base;
        return (jclass)klassAddr;
    }
    else {
        jclass klass = NULL;
        if (!ApiReadmem(&klass, PTRMATH((uintptr_t)obj + klassOffset), sizeof(jclass))) {
            return NULL;
        }
        return klass;
    }
}

jboolean IsInstanceOf(jobject obj, jclass clazz) {
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

void MonitorEnter(jobject obj) {
    (void)obj;
}

void MonitorExit(jobject obj) {
    (void)obj;
}

void SetObjectField(jobject obj, jfieldID fieldID, jobject value) {
    SetFieldObject(obj, fieldID, value);
}

jboolean IsSameObject(jobject obj1, jobject obj2) {
    return obj1 == obj2 ? 1 : 0;
}

jclass GetSuperclass(jclass clazz) {
    if (!clazz) return NULL;
    jclass superClass = NULL;
    ApiReadmem(&superClass, PTRMATH((uintptr_t)clazz + (uintptr_t)SuperEntry->offset), sizeof(jclass));
    return superClass;
}

jboolean IsAssignableFrom(jclass sub, jclass super) {
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

jobject AllocObject(jclass clazz) {
    (void)clazz;
    return NULL;
}

SJStatus ApiNewJvmInterface(In_ HotspotContext Context, Out_ PIJVMINTERFACE* Interface)
{
    ActivityContext = &Context;
    jvm = Context._proc->hJvmDll;

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