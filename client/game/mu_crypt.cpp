#include "stdafx.h"
#include "mu_crypt.h"

#include <tea.h>
#include <3way.h>
#include <cast.h>
#include <rc5.h>
#include <rc6.h>
#include <mars.h>
#include <idea.h>
#include <gost.h>

class MCryptoCipherBase
{
public:
	virtual bool Initialize(const mu_uint8 *key, const mu_uint32 keylength) = 0;
	virtual mu_uint32 Encrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output) = 0;
	virtual mu_uint32 Decrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output) = 0;
	virtual mu_uint32 GetBlockSize() = 0;
};

template<typename Crypto, const mu_uint32 buffer = 8192, const mu_uint32 rounds = 1024>
class MCryptoCipher : public MCryptoCipherBase
{
public:
	virtual bool Initialize(const mu_uint8 *key, const mu_uint32 keylength) override
	{
		CipherEncryption.SetKey(key, CipherEncryption.DEFAULT_KEYLENGTH);
		CipherDecryption.SetKey(key, CipherDecryption.DEFAULT_KEYLENGTH);
		return true;
	}

	virtual mu_uint32 Encrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output) override
	{
		if (output == nullptr || inputLength <= 0) return inputLength;

		mu_uint32 bufferSize = inputLength - (inputLength % CipherEncryption.BLOCKSIZE);
		for (mu_uint32 n = 0; n < bufferSize; n += CipherEncryption.BLOCKSIZE)
		{
			CipherEncryption.ProcessAndXorBlock(input + n, nullptr, output + n);
		}

		if (bufferSize < inputLength)
		{
			mu_memcpy(output + bufferSize, input + bufferSize, inputLength - bufferSize);
		}

		return inputLength;
	}

	virtual mu_uint32 Decrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output) override
	{
		if (output == nullptr || inputLength <= 0) return inputLength;

		mu_uint32 bufferSize = inputLength - (inputLength % CipherDecryption.BLOCKSIZE);
		for (mu_uint32 n = 0; n < bufferSize; n += CipherDecryption.BLOCKSIZE)
		{
			CipherDecryption.ProcessAndXorBlock(input + n, nullptr, output + n);
		}

		if (bufferSize < inputLength)
		{
			mu_memcpy(output + bufferSize, input + bufferSize, inputLength - bufferSize);
		}

		return inputLength;
	}

	virtual mu_uint32 GetBlockSize() override
	{
		return CipherEncryption.BLOCKSIZE;
	}

private:
	typename Crypto::Encryption CipherEncryption;
	typename Crypto::Decryption CipherDecryption;
};

constexpr mu_uint32 TotalCiphers = 8;
const char *key = "webzen#@!01webzen#@!01webzen#@!0";
const mu_uint32 keyLength = static_cast<mu_uint32>(strlen(key));

class MCryptoManager
{
public:
	MCryptoManager() : AlgorithmType(NInvalidUInt32), Algorithm(nullptr)
	{
	}

	~MCryptoManager()
	{
		Destroy();
	}

	void Initialize(mu_uint8 algorithm, const mu_uint8 *key, const mu_uint32 keylength)
	{
		Destroy();

		AlgorithmType = algorithm % TotalCiphers;
		switch (AlgorithmType)
		{
		case 0:
			{
				Algorithm = new MCryptoCipher<CryptoPP::TEA>();
			}
			break;

		case 1:
			{
				Algorithm = new MCryptoCipher<CryptoPP::ThreeWay>();
			}
			break;

		case 2:
			{
				Algorithm = new MCryptoCipher<CryptoPP::CAST128>();
			}
			break;

		case 3:
			{
				Algorithm = new MCryptoCipher<CryptoPP::RC5>();
			}
			break;

		case 4:
			{
				Algorithm = new MCryptoCipher<CryptoPP::RC6>();
			}
			break;

		case 5:
			{
				Algorithm = new MCryptoCipher<CryptoPP::MARS>();
			}
			break;

		case 6:
			{
				Algorithm = new MCryptoCipher<CryptoPP::IDEA>();
			}
			break;

		case 7:
			{
				Algorithm = new MCryptoCipher<CryptoPP::GOST>();
			}
			break;
		}

		if (Algorithm != nullptr)
		{
			Algorithm->Initialize(key, keylength);
		}
	}

	void Destroy()
	{
		if (Algorithm != nullptr)
		{
			delete Algorithm;
			Algorithm = nullptr;
		}
	}

	mu_uint32 Encrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output)
	{
		return Algorithm->Encrypt(input, inputLength, output);
	}

	mu_uint32 Decrypt(const mu_uint8 *input, const mu_uint32 inputLength, mu_uint8 *output)
	{
		return Algorithm->Decrypt(input, inputLength, output);
	}

	mu_uint32 GetBlockSize()
	{
		return Algorithm->GetBlockSize();
	}

	mu_uint32 GetBufferLength(const mu_uint32 BufferSize)
	{
		if (BufferSize == 0) return 0;

		return BufferSize - (BufferSize % GetBlockSize());
	}

private:
	mu_uint32 AlgorithmType;
	MCryptoCipherBase *Algorithm;
};

struct CryptoModulusHeader
{
public:
	mu_uint8 Algorithms[2];
	mu_uint8 Key[32];
};

mu_uint32 CryptoModulusDecrypt(mu_uint8 *inputBuffer, const mu_uint32 inputLength, mu_uint8 *outputBuffer)
{
	if (outputBuffer == nullptr)
		return inputLength - sizeof(CryptoModulusHeader);

	MCryptoManager crypto;
	crypto.Initialize(inputBuffer[1], reinterpret_cast<const mu_uint8 *>(key), keyLength);

	mu_uint32 realSize = inputLength - sizeof(CryptoModulusHeader);
	mu_uint32 bufferSize = crypto.GetBufferLength(1024);

	if (realSize > sizeof(mu_uint32) * bufferSize)
	{
		const mu_uint32 index = realSize / 2 + 2;
		crypto.Decrypt(inputBuffer + index, bufferSize, inputBuffer + index);
	}

	if (realSize > bufferSize)
	{
		const mu_uint32 index = inputLength - bufferSize;
		crypto.Decrypt(inputBuffer + index, bufferSize, inputBuffer + index);
		crypto.Decrypt(inputBuffer + 2, bufferSize, inputBuffer + 2);
	}

	crypto.Initialize(inputBuffer[0], inputBuffer + 2, 32);
	bufferSize = crypto.GetBufferLength(realSize);
	if (inputBuffer != outputBuffer)
	{
		crypto.Decrypt(inputBuffer + sizeof(CryptoModulusHeader), realSize, outputBuffer);
	}
	else
	{
		crypto.Decrypt(inputBuffer + sizeof(CryptoModulusHeader), realSize, outputBuffer + sizeof(CryptoModulusHeader));
	}

	return realSize;
}