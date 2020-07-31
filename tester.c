#include <stdlib.h>
#include <stdio.h>

#include "sigslot.h"

void _slot1(int a, int b, int c) {
    printf("slot1 : %d %d %d \n", a, b, c);
}

void _slot2(int a, int b, int c) {
    printf("slot2 : %d %d %d \n", a, b, c);
}

void _slot3(int a, int b, int c) {
    printf("slot3 : %d %d %d \n", a, b, c);
}

typedef struct _t_signal_slot {
    c_signal(signal1);
    c_signal(signal2);
} signal_tester_t;

int main(int argc, char *argv[]) {
    printf("%s build time : %s %s\n\n", _program_name(), __DATE__, __TIME__);
    printf("----------------------------------\n");

    signal_tester_t tester;
    c_signal_initialize(&tester, signal1);
    c_signal_initialize(&tester, signal2);
    printf("----------------------------------\n");

    c_signal_connector(&tester, signal1, _slot1);
    printf("slot1 is connected to signal : %d \n", c_signal_connected(&tester, signal1, _slot1));
    printf("slot2 is connected to signal : %d \n", c_signal_connected(&tester, signal1, _slot2));
    printf("----------------------------------\n");

    c_signal_connector(&tester, signal1, _slot1);
    c_signal_connector(&tester, signal1, _slot2);
    c_signal_connector(&tester, signal1, _slot3);
    c_signal_emit(&tester, signal1, void(*)(int, int, int), 1, 2, 3);
    printf("----------------------------------\n");

    c_signal_disconnect(&tester, signal1, _slot1);
    c_signal_emit(&tester, signal1, void(*)(int, int, int), 4, 5, 6);
    printf("----------------------------------\n");

    c_signal_disconnect(&tester, signal1, _slot3);
    c_signal_emit(&tester, signal1, void(*)(int, int, int), 7, 8, 9);
    printf("----------------------------------\n");

    c_signal_connector(&tester, signal1, _slot3);
    c_signal_connector(&tester, signal1, _slot1);
    c_signal_emit(&tester, signal1, void(*)(int, int, int), 0, 0, 0);
    printf("----------------------------------\n");

    c_signal_uninitialize(&tester, signal1);
    c_signal_uninitialize(&tester, signal2);
    printf("----------------------------------\n");

    printf("\n");
    return EXIT_SUCCESS;
}
