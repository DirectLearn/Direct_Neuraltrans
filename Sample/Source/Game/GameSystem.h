#pragma once


//----------------------------------
//GameSystemクラス
//このゲームの土台となるもの
//--------------------------------
class GameSystem
{
public:
	//このゲームの初期設定を行う
	void Initialize();

	//このゲームの世界の時間を進める
	void Execute();

	//その他、ゲーム用のデータなどをここに書く


//----------------------------------------------
//今回このクラスも、どこからでもアクセスできるように
//シングルトンパターンにする
//-----------------------------------------------
private:
	//唯一のインスタンス用のポインタ
	static inline GameSystem* s_instance;
	//コンストラクタはprivateにする
	GameSystem(){}
public:
	//インスタンス作成
	static void CreateInstance()
	{
		DeleteInstance();

		s_instance = new GameSystem();
	}
	//インスタンス削除
	static void DeleteInstance()
	{
		if (s_instance != nullptr)
		{
			delete s_instance;
			s_instance = nullptr;
		}
	}
	//唯一のインスタンスを取得
	static GameSystem& GetInstance()
	{
		return *s_instance;
	}
};

//GameSystemの唯一のインスタンスを簡単に取得するためのマクロ
#define GAMESYS GameSystem::GetInstance()