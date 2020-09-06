#include "headers/encryption.h"
#include <QDebug>

using namespace CryptoPP;

std::string Encryption::encrypt(std::string content, std::string passphrase) {
    AutoSeededRandomPool prng;
    byte iv[AES::BLOCKSIZE];
    prng.GenerateBlock(iv, sizeof(iv));
    std::string ivString(reinterpret_cast< char const* >(iv));
    SecByteBlock key(AES::MAX_KEYLENGTH + AES::BLOCKSIZE);

    std::string encrypted, output;

    try {
        HKDF<SHA256> hkdf;
        hkdf.DeriveKey(key, key.size(), (const byte*) passphrase.data(), passphrase.size(), (const byte*) ivString.c_str(), AES::BLOCKSIZE, NULL, 0);

        CTR_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV(key, AES::MAX_KEYLENGTH, key+AES::MAX_KEYLENGTH);

        StringSource(content, true, new StreamTransformationFilter(encryption, new StringSink(encrypted)));

        std::string finalStr = "encrypted" + ivString + "__ENDIV" + encrypted;

        StringSource(finalStr, true, new Base64Encoder(new StringSink(output)));
    } catch (const Exception& ex) {
        qDebug() << ex.what();
    }

    return  output;
}

std::string Encryption::decrypt(std::string content, std::string passphrase) {
    std::string base64Decoded, ivString, decrypted;

    StringSource(content, true, new Base64Decoder(new StringSink(base64Decoded)));

    if (base64Decoded.substr(0, ENCRYPTED_LENGTH) != "encrypted")
        return "";

    unsigned first = base64Decoded.find("encrypted") + ENCRYPTED_LENGTH;
    unsigned last = base64Decoded.find("__ENDIV");
    ivString = base64Decoded.substr(first, last - first);

    first = base64Decoded.find("__ENDIV") + ENDIV_LENGTH;

    base64Decoded = base64Decoded.substr(first);

    SecByteBlock key(AES::MAX_KEYLENGTH + AES::BLOCKSIZE);

    try {
        HKDF<SHA256> hkdf;
        hkdf.DeriveKey(key, key.size(), (const byte*)passphrase.data(), passphrase.size(), (const byte*) ivString.c_str(), AES::BLOCKSIZE, NULL, 0);

        CTR_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV(key, AES::MAX_KEYLENGTH, key + AES::MAX_KEYLENGTH);

        StringSource(base64Decoded, true, new StreamTransformationFilter(decryption, new StringSink(decrypted)));
    } catch (const Exception& ex) {
        qDebug() << ex.what();
    }

    return decrypted;
}

bool Encryption::isEncrypted(std::string content) {
    std::string base64Decoded;
    StringSource(content, true, new Base64Decoder(new StringSink(base64Decoded)));
    return (base64Decoded.substr(0, ENCRYPTED_LENGTH) == "encrypted");
}
