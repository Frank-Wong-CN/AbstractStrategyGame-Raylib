#define CLASS(X) class X : public Object
#define BEGIN_FUNC(X) X() : Object()
#define FUNC(X) X = [this]()
#define BEGIN_ALARM(N) alarm.push_back((float)(N)); Alarms.emplace_back([this](){
#define END_ALARM });
//#define BEGIN_COL(O) Collisions.insert_or_assign({ (O), [this](){
//#define END_COL }});

#define DEBUG printf("[DEBUG]\n");
#define DEBUGI(X) printf("[DEBUG] Output: %d.\n", (X))
#define DEBUGF(X) printf("[DEBUG] Output: %.2f.\n", (X))
#define DEBUGS(...) printf("[DEBUG]" __VA_ARGS__)

#define STARTING_CTIMER 0.8f
#define STARTING_HP 100.0f
#define MATTER_EFFECTIVE_HP 5.0f

#define ACTIVE_LAYER_RANGE 1
#define DISABLE_SIBLING false
#define NODE_VERTICAL_SPACING 35.0f
#define NODE_HORIZONTAL_SPACING 50.0f
#define NODE_RADIUS 60.0f
#define NODE_GEN_RANGE_MIN 2
#define NODE_GEN_RANGE_MAX 5
#define NODE_PERCEP_TEXT_COLOR BLACK
#define NODE_REGEN_TEXT_COLOR WHITE
#define NODE_DESTR_TEXT_COLOR BLACK
#define NODE_PROTEC_TEXT_COLOR GREEN
#define NODE_PERCEP_COLOR WHITE
#define NODE_REGEN_COLOR GREEN
#define NODE_DESTR_COLOR RED
#define NODE_PROTEC_COLOR BLUE
#define NODE_CONF_DIR "res\\"
#define NODE_PERCEP_FILE "percep.txt"
#define NODE_REGEN_FILE "regen.txt"
#define NODE_DESTR_FILE "destr.txt"
#define NODE_PROTEC_FILE "protec.txt"

#define NET_PORT 2203
#define NET_PASSWD "GNSASG39748"
#define VERIFY_PADDING "\x11\x22\x33\x44"
#define VERIFY_PADDING_SIZE 4

void dump(char *desc, void *addr, int len) 
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

#include "ThreadSupport.hpp"
#include "ClassDef.hpp"
#include "ObjectDef.hpp"

#include "Classes/Decision.hpp"
#include "Classes/DecisionTree.hpp"
#include "Classes/NodeFactory.hpp"
#include "Classes/Information.hpp"
#include "Classes/GameController.hpp"
