#include "os.h"

MOCK_BOLOS_FN(os_perso_derive_node_bip32, 10, {
    unsigned char *privkey;
    int key_len;
    unsigned char *chain;
    int chain_len;
})
void os_perso_derive_node_bip32(cx_curve_t curve,
                                const unsigned int *path,
                                unsigned int pathLength,
                                unsigned char *privateKey,
                                unsigned char *chain) {
    MOCK_FN_INTERNAL(os_perso_derive_node_bip32);
    memmove(privateKey, response->privkey, response->key_len);
    memmove(chain, response->chain, response->chain_len);
}
