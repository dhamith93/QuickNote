#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include "osrng.h"
#include "cryptlib.h"
#include "aes.h"
#include "base64.h"
#include "sha.h"
#include "hkdf.h"
#include "modes.h"
#include "filters.h"


class Encryption {
    private:
        const static int ENCRYPTED_LENGTH = 9;
        const static int ENDIV_LENGTH = 7;
    public:
        static std::string encrypt(std::string content, std::string passphrase);
        static std::string decrypt(std::string content, std::string passphrase);
        static bool isEncrypted(std::string content);
};

#endif // ENCRYPTION_H
