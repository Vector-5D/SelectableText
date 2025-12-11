# Selectable Text for C
A small single-header C library for UNIX operating systems to make simple and easy CLI interfaces.

# Usage
Download the header file and include it. Below is a usage example.
```
#include <stdio.h>
#include "selectabletext.h"

void action_a() { puts("Selected action one."); }
void action_b() { puts("Selected action two."); } 
void action_c() { puts("Selected action three."); } 

int main() {
    // text surrounded by backticks ` will be parsed to selectable actions
    char* fmt_text =
        "Please select an action.\n"
        "`[1]` Action one\n"
        "`[2]` Action two\n"
        "`[3]` Action three\n";

    SelectableText stext = {0};

    // parses the format text and defines the fields in stext
    selectableText_loadFmtString(fmt_text, &stext);

    // returns the index of the selected action
    size_t result = selectableText_promptUser(&stext);
    
    selectableText_free(&stext);

    switch (result) {
        case 0:
            action_a();
            break;
        case 1:
            action_b();
            break;
        case 2:
            action_c();
            break;
        default:
            // this path shouldn't be reachable
            puts("Something went wrong.");
            break;
    }

    // free stext
    selectableText_free(&stext);
   
    return 0;
}
```

# License
This library is provided as is. The author(s) take no responsibility for any possible damages caused by this library.
