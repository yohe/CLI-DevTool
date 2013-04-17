
#include <test_case.hpp>
#include <test_macros.hpp>

#include "key_seq.h"
#include "key_code.h"

using namespace iunit;

class KeySequenceEntryTest : public CppTestCase {
    KeySequenceEntry* entry;
public:
    KeySequenceEntryTest() : CppTestCase("KeySequenceEntryTest") {
        entry = NULL;
    }

    virtual void setup() {
        entry = new KeySequenceEntry(1, KeyCode::KEY_DEL);
    }
    virtual void teardown() {
        delete entry; entry = NULL;
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeySequenceEntryTest, test);
    }
};

void KeySequenceEntryTest::test() {
    IUNIT_EQ(1, entry->getSequenceCode());
    IUNIT_EQ(KeyCode::KEY_DEL, entry->getVirtualKeyCode());
    //IUNIT_EQ("CTRL-X", entry->getStrokeName());
    IUNIT_EQ(true, entry->isEntry());
    IUNIT_EQ(NULL, entry->getKeySequenceEntry(2));
    IUNIT_EQ(entry, entry->getKeySequenceEntry(1));
}

class KeySequenceGroupTest : public CppTestCase {
    KeySequenceGroup* group;
    KeySequenceEntry* entry1;
    KeySequenceEntry* entry2;
public:
    KeySequenceGroupTest() : CppTestCase("KeySequenceGroupTest") {}

    virtual void setup() {
        group = new KeySequenceGroup(50);
        entry1 = new KeySequenceEntry(1, KeyCode::KEY_CTRL_J);
        entry2 = new KeySequenceEntry(2, KeyCode::KEY_CTRL_J);
        group->addKeySequence(entry1);
        group->addKeySequence(entry2);
    }
    virtual void teardown() {
        delete group;
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeySequenceGroupTest, test);
    }
};

void KeySequenceGroupTest::test() {
    IUNIT_EQ(50, group->getSequenceCode());
    //IUNIT_EQ(-1, entry->getActionCode());
    //IUNIT_EQ("null", entry->getStrokeName());
    IUNIT_EQ(false, group->isEntry());
    IUNIT_EQ(entry1, entry1->getKeySequenceEntry(1));
    IUNIT_EQ(NULL, entry1->getKeySequenceEntry(2));
    IUNIT_EQ(entry2, entry2->getKeySequenceEntry(2));
    IUNIT_EQ(NULL, group->getKeySequenceEntry(50));

    IUNIT_EQ(entry1, group->getKeySequenceEntry(1));
    IUNIT_EQ(entry2, group->getKeySequenceEntry(2));
}

