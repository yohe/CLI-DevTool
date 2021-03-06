
#include <test_case.hpp>
#include <test_macros.hpp>

#include "key/key_map.h"
#include "key/key_code.h"

using namespace iunit;
using namespace clidevt;

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
#define ADD_KEY_MAP(code, strokeList) \
    strokeList; \
    keyMap.addKeyCodeSeq(code, stroke); \
    stroke.clear();

class KeyMapTest : public CppTestCase {
    KeyMap keyMap;
public:
    KeyMapTest() : CppTestCase("KeyMapTest") {
    }

    virtual void setup() {
        std::vector<char> stroke;
        ADD_KEY_MAP( KeyCode::KEY_CTRL_A, KEY_STROKE_DEF(1, (1)));
        ADD_KEY_MAP( KeyCode::KEY_UP_ARROW, KEY_STROKE_DEF(3, (27) (91) (65)));
        ADD_KEY_MAP( KeyCode::KEY_DOWN_ARROW, KEY_STROKE_DEF(3, (27) (91) (66)));
    }
    virtual void teardown() {
        keyMap.deleteKeyCodeSeq(KeyCode::KEY_UP_ARROW);
        keyMap.deleteKeyCodeSeq(KeyCode::KEY_CTRL_A);
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeyMapTest, test);
    }
};

void KeyMapTest::test() {
    IUNIT_FALSE(KeyCode::KEY_UP_ARROW == KeyCode::KEY_DOWN_ARROW);
    IUNIT_NOT_NULL(keyMap.getKeyEntry(1));
    IUNIT_NULL(keyMap.getKeyEntry(2));
    const KeySequenceEntry* entry = keyMap.getKeyEntry(1);
    IUNIT_EQ(true, entry->isEntry());
    entry = keyMap.getKeyEntry(27);
    IUNIT_EQ(false, entry->isEntry());
    entry = entry->getKeySequenceEntry(91);
    IUNIT_NOT_NULL( entry );
    IUNIT_EQ(false, entry->isEntry());
    const KeySequenceEntry* tmp = entry->getKeySequenceEntry(65);
    IUNIT_NOT_NULL( tmp );
    IUNIT_EQ(true, tmp->isEntry());
    tmp = entry->getKeySequenceEntry(66);
    IUNIT_NOT_NULL( tmp );
    IUNIT_EQ(true, tmp->isEntry());
    keyMap.deleteKeyCodeSeq(KeyCode::KEY_CTRL_A);
    IUNIT_NOT_NULL(keyMap.getKeyEntry(27));
    IUNIT_NULL(keyMap.getKeyEntry(1));
}

