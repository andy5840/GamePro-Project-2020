#include "stdafx.h"
#include "Audio.h"


Audio::Audio() : xaudio2(nullptr), masterVoice(nullptr)
{
}


Audio::~Audio()
{
}

bool Audio::Initialize()
{
	if (FAILED(MFStartup(MF_VERSION)))
	{
		printf("WMF �ʱ�ȭ ����\n");
		return false;
	}

	if (FAILED(MFCreateAttributes(&sourceReaderConfiguration, 1)))
	{
		printf("WMF �ҽ����� �Ӽ� ���� ����\n");
		return false;
	}
	if (FAILED(sourceReaderConfiguration->SetUINT32(MF_LOW_LATENCY, true)))
	{
		printf("WMF �ҽ����� �Ӽ� ���� ����\n");
		return false;
	}

	if (FAILED(XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		printf("����� �ʱ�ȭ ����, XAudio2Create\n");
		return false;
	}
	if (FAILED(xaudio2->CreateMasteringVoice(&masterVoice)))
	{
		printf("����� �ʱ�ȭ ����, CreateMasteringVoice\n");
		return false;
	}
	return true;
}

void Audio::Uninitialize()
{
	MFShutdown();
	masterVoice->DestroyVoice();
	xaudio2->StopEngine();
}

bool Audio::LoadFile(const wchar_t* path, std::vector<BYTE>& audioData, WAVEFORMATEX** waveFormatEx, unsigned int& waveLength)
{
	//WMF�� Ȱ���Ͽ� ���� ���ڵ�
	HRESULT hr = S_OK;

	IMFSourceReader* sourceReader;
	hr = MFCreateSourceReaderFromURL(path, sourceReaderConfiguration, &sourceReader);	//1�� �ε�(7mb)
	if (FAILED(hr))
	{
		printf("�ҽ����� ���� ����, ����� ������ ��θ� Ȯ�����ּ���\n");
		return false;
	}

	//����� ��Ʈ���� ���
	DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;
	hr = sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
	if (FAILED(hr))
	{
		printf("��Ʈ�� ���� ����\n");
		return false;
	}
	hr = sourceReader->SetStreamSelection(streamIndex, true);
	if (FAILED(hr))
	{
		printf("��Ʈ�� ���� ����\n");
		return false;
	}
	//�̵��Ÿ�� Ȯ��
	IMFMediaType* nativeMediaType;
	hr = sourceReader->GetNativeMediaType(streamIndex, 0, &nativeMediaType);
	if (FAILED(hr))
	{
		printf("�̵�� Ÿ�� �ĺ� ����\n");
		return false;
	}

	//������ ����� Ÿ������ Ȯ��
	GUID majorType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
	if (majorType != MFMediaType_Audio) {
		printf("����� ������ �ƴմϴ�.\n");
		return false;
	}

	// ������ ���������, �ƴ��� Ȯ��
	GUID subType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
	if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM)
	{
		// the audio file is uncompressed
	}
	else
	{
		// the audio file is compressed; we have to decompress it first

		IMFMediaType* partialType = nullptr;
		hr = MFCreateMediaType(&partialType);
		if (FAILED(hr))
		{
			printf("�̵�� Ÿ�� ���� ����\n");
			return false;
		}

		hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		if (FAILED(hr))
		{
			puts("�̵��Ÿ���� ������� �����ϴµ� �����Ͽ����ϴ�.");
			return false;
		}

		hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		if (FAILED(hr))
		{
			puts("����Ÿ���� PCM���� �����ϴµ� �����Ͽ����ϴ�.");
			return false;
		}

		hr = sourceReader->SetCurrentMediaType(streamIndex, NULL, partialType);
		if (FAILED(hr))
		{
			puts("���� �̵��Ÿ�� ���� ����");
			return false;
		}
	}

	//���� ���ڵ��� ���� �����̴� �ε� ����
	//XAudio2 ���۷� �̵��ϱ� ���� �����͸� ���� �����ϰ� �ε�
	//FAILED ����ó�� ���ֱ�
	IMFMediaType* uncompressedAudioType = nullptr;
	hr = sourceReader->GetCurrentMediaType(streamIndex, &uncompressedAudioType);
	if (FAILED(hr))
	{
		printf("erroccured\n");
		return false;
	}

	hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType, waveFormatEx, &waveLength);
	if (FAILED(hr))
	{
		printf("wave foramt �ε� ����\n");
		return false;
	}


	//��Ʈ�� ����
	hr = sourceReader->SetStreamSelection(streamIndex, true);
	if (FAILED(hr))
	{
		printf("erroccured\n");
		return false;
	}

	//�����͸� vector�� �ű�ϴ�.
	IMFSample* sample = nullptr;
	IMFMediaBuffer* buffer = nullptr;
	BYTE* localAudioData = NULL;
	DWORD localAudioDataLength = 0;

	int count = 0;
	while (true)
	{
		DWORD flags = 0;
		hr = sourceReader->ReadSample(streamIndex, 0, nullptr, &flags, nullptr, &sample);
		if (FAILED(hr))
		{
			printf("erroccured\n");
			return false;
		}

		//�����Ͱ� ������ ��� �������� üũ
		if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
			break;

		//������ �� üũ
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;
		if (sample == nullptr)
			continue;

		//�����͸� ���۷� ��ȯ
		hr = sample->ConvertToContiguousBuffer(&buffer);
		if (FAILED(hr))
		{
			printf("erroccured\n");
			return false;
		}

		//���۸� ��װ�, �����͸� �޸𸮷� ����
		hr = buffer->Lock(&localAudioData, nullptr, &localAudioDataLength);
		if (FAILED(hr))
		{
			printf("erroccured\n");
			return false;
		}

		for (size_t i = 0; i < localAudioDataLength; i++)
			audioData.push_back(localAudioData[i]);

		//���� ��� ����
		hr = buffer->Unlock();
		if (FAILED(hr))
		{
			printf("erroccured\n");
			return false;
		}
		localAudioData = nullptr;
		SAFE_RELEASE(sample);
		SAFE_RELEASE(buffer);
		count++;
	}
	return true;
}

Audio* Audio::GetInstance()
{
	static Audio a;
	return &a;
}