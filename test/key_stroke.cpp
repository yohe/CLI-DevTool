
#include <test_case.hpp>
#include <test_macros.hpp>

#include "key_stroke.h"

using namespace iunit;

class KeyStrokeEntryTest : public CppTestCase {
    KeyStrokeEntry* entry;
public:
    KeyStrokeEntryTest() : CppTestCase("KeyStrokeEntryTest") {
        entry = NULL;
    }

    virtual void setup() {
        entry = new KeyStrokeEntry(1, "CTRL-X", 100);
    }
    virtual void teardown() {
        delete entry; entry = NULL;
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeyStrokeEntryTest, test);
    }
};

void KeyStrokeEntryTest::test() {
    IUNIT_EQ(1, entry->getKeyCode());
    IUNIT_EQ(100, entry->getActionCode());
    IUNIT_EQ("CTRL-X", entry->getStrokeName());
    IUNIT_EQ(true, entry->isEntry());
    IUNIT_EQ(NULL, entry->getKeyStroke(2));
    IUNIT_EQ(entry, entry->getKeyStroke(1));
}

class KeyStrokeGroupTest : public CppTestCase {
    KeyStrokeGroup* group;
    KeyStrokeEntry* entry;
public:
    KeyStrokeGroupTest() : CppTestCase("KeyStrokeGroupTest") {}

    virtual void setup() {
        group = new KeyStrokeGroup(50);
        entry = new KeyStrokeEntry(1, "CTRL-X", 10);
        group->addKeyStroke(entry);
    }
    virtual void teardown() {
        delete group;
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( KeyStrokeGroupTest, test);
    }
};

void KeyStrokeGroupTest::test() {
    IUNIT_EQ(50, group->getKeyCode());
    //IUNIT_EQ(-1, entry->getActionCode());
    //IUNIT_EQ("null", entry->getStrokeName());
    IUNIT_EQ(false, group->isEntry());
    IUNIT_EQ(NULL, entry->getKeyStroke(2));
    IUNIT_EQ(NULL, entry->getKeyStroke(50));
    IUNIT_EQ(entry, entry->getKeyStroke(1));
}

