#pragma once


//----------------------------------
//GameSystem�N���X
//���̃Q�[���̓y��ƂȂ����
//--------------------------------
class GameSystem
{
public:
	//���̃Q�[���̏����ݒ���s��
	void Initialize();

	//���̃Q�[���̐��E�̎��Ԃ�i�߂�
	void Execute();

	//���̑��A�Q�[���p�̃f�[�^�Ȃǂ������ɏ���


//----------------------------------------------
//���񂱂̃N���X���A�ǂ�����ł��A�N�Z�X�ł���悤��
//�V���O���g���p�^�[���ɂ���
//-----------------------------------------------
private:
	//�B��̃C���X�^���X�p�̃|�C���^
	static inline GameSystem* s_instance;
	//�R���X�g���N�^��private�ɂ���
	GameSystem(){}
public:
	//�C���X�^���X�쐬
	static void CreateInstance()
	{
		DeleteInstance();

		s_instance = new GameSystem();
	}
	//�C���X�^���X�폜
	static void DeleteInstance()
	{
		if (s_instance != nullptr)
		{
			delete s_instance;
			s_instance = nullptr;
		}
	}
	//�B��̃C���X�^���X���擾
	static GameSystem& GetInstance()
	{
		return *s_instance;
	}
};

//GameSystem�̗B��̃C���X�^���X���ȒP�Ɏ擾���邽�߂̃}�N��
#define GAMESYS GameSystem::GetInstance()