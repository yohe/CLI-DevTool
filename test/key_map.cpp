
#include <test_case.hpp>
#include <test_macros.hpp>

#include "key_map.h"
#include "action_code.h"

using namespace iunit;

#define SEMICOLON_SPLIT_1(x) stroke.push_back(x);
#define SEMICOLON_SPLIT_2(x) stroke.push_back(x); SEMICOLON_SPLIT_1
#define SEMICOLON_SPLIT_3(x) stroke.push_back(x); SEMICOLON_SPLIT_2
#define SEMICOLON_SPLIT_4(x) stroke.push_back(x); SEMICOLON_SPLIT_3

// KEY_STROKE_DEF は Num に Asciiコード列長, Seq に Asciiコード列 を記載
// Asciiコード列は、 (first) (second) (therd) のように各コードを () で括り記載する
// Asciiコード列の調査は、make 時に作成される key_trace を使用する
// 0x20 - 0x7F の間のAsciiコード( alphanumeric symbol )で始まるコード列は使用できません
#define KEY_STROKE_DEF(Num, Seq) \
    SEMICOLON_SPLIT_##Num Seq;

// strokeListは KEY_STROKE_DEF を使用
#define ADD_KEY_MAP(name, code, strokeList) \
    strokeList; \
    keyMap.addKeyStroke(name, stroke, code); \
    stroke.clear();

class KeyMapTest : public CppTestCase {
    KeyMap keyMap;
public:
    KeyMapTest() : CppTestCase("KeyMapTest") {
    }

    virtual void setup() {
        std::vector<char> stroke;
        ADD_KEY_MAP("CTRL-A", ActionCode::KEY_CTRL_A, KEY_STROKE_DEF(1, (1)));
        ADD_KEY_MAP("UP", ActionCode::KEY_UP_ARROW, KEY_STROKE_DEF(3, (27) (91) (65)));
    }
    virtual void teardown() {
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeyMapTest, test);
    }
};

void KeyMapTest::test() {
    IUNIT_NOT_NULL(keyMap.getKeyEntry(1));
    IUNIT_NULL(keyMap.getKeyEntry(2));
    KeyStrokeEntry* entry = keyMap.getKeyEntry(1);
    IUNIT_EQ(true, entry->isEntry());
    entry = keyMap.getKeyEntry(27);
    IUNIT_EQ(false, entry->isEntry());
    keyMap.deleteKeyStroke("UP");
    IUNIT_NULL(keyMap.getKeyEntry(3));
}

