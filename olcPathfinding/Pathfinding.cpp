#include "olcPixelGameEngine.h"

struct cell {
	int32_t x = -1;
	int32_t y = -1;
	int32_t costFactor = 1;
	boolean isObstacle = false;
	boolean isStart = false;
	boolean isEnd = false;
};

// Pathfinding
class Pathfinding : public olc::PixelGameEngine
{
public:
	Pathfinding()
	{
		// Name you application
		sAppName = "Pathfinding";
	}

private:
	int32_t nCellSize;
	int32_t nBorderSize;
	int32_t nAreaHeight;
	int32_t nAreaWidth;
	cell* cBoard;
	int32_t nMouseCellXLastFrame;
	int32_t nMouseCellYLastFrame;
	boolean bCalculate;
	int32_t nStartX;
	int32_t nStartY;
	int32_t nEndX;
	int32_t nEndY;
	std::list<cell> cPath;
	int32_t* nHeightMap;
	int32_t nMaxSteps;

public:
	bool OnUserCreate() override
	{
		nCellSize = 24;
		nBorderSize = 2;
		nAreaHeight = ScreenHeight() / nCellSize;
		nAreaWidth = ScreenWidth() / nCellSize;
		cBoard = new cell[nAreaWidth * nAreaHeight];
		nHeightMap = new int[nAreaWidth * nAreaHeight];
		nMaxSteps = 15;

		for (int x = 0; x < nAreaWidth; x++)
			for (int y = 0; y < nAreaHeight; y++)
			{
				cell* cur = &cBoard[y * nAreaWidth + x];

				cur->x = x;
				cur->y = y;
				cur->isObstacle = (x == 0 || x == nAreaWidth - 1 || y == 0 || y == nAreaHeight - 1);

				nHeightMap[y * nAreaWidth + x] = cur->isObstacle ? -1 : 0;
			}

		nStartX = 1 + (rand() % (nAreaWidth - 3));
		nStartY = 1 + (rand() % (nAreaHeight - 3));
		cBoard[nStartY * nAreaWidth + nStartX].isStart = true;

		nEndX = 1 + (rand() % (nAreaWidth - 3));
		nEndY = 1 + (rand() % (nAreaHeight - 3));
		cBoard[nEndY * nAreaWidth + nEndX].isEnd = true;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Helper lambda
		auto xy = [&](int x, int y) {return y * nAreaWidth + x; };
		auto toggleObstacle = [&](int p) {
			if (!cBoard[p].isStart && !cBoard[p].isEnd) {
				cBoard[p].isObstacle = !cBoard[p].isObstacle;
			}
		};
		auto setStart = [&](int x, int y) {
			if (cBoard[xy(x, y)].isObstacle || cBoard[xy(x, y)].isEnd || cBoard[xy(x, y)].isStart)
				return;

			cBoard[xy(x, y)].isStart = true;
			cBoard[xy(nStartX, nStartY)].isStart = false;

			nStartX = x;
			nStartY = y;
			bCalculate = true;
		};
		auto setEnd = [&](int x, int y) {
			if (cBoard[xy(x, y)].isObstacle || cBoard[xy(x, y)].isStart || cBoard[xy(x, y)].isEnd)
				return;

			cBoard[xy(x, y)].isEnd = true;
			cBoard[xy(nEndX, nEndY)].isEnd = false;

			nEndX = x;
			nEndY = y;
			bCalculate = true;
		};

		// Prepare
		int nMouseCellX = GetMouseX() / nCellSize;
		int nMouseCellY = GetMouseY() / nCellSize;

		// Logic
		if (bCalculate)
			CalculatePath(GetKey(olc::Key::SHIFT).bHeld);

		if (GetMouse(0).bPressed)
			toggleObstacle(xy(nMouseCellX, nMouseCellY));

		if (GetMouse(0).bHeld && (xy(nMouseCellX, nMouseCellY) != xy(nMouseCellXLastFrame, nMouseCellYLastFrame)))
			toggleObstacle(xy(nMouseCellX, nMouseCellY));

		if (GetMouse(0).bReleased)
			bCalculate = true;

		if (GetKey(olc::Key::S).bReleased)
			setStart(nMouseCellX, nMouseCellY);

		if (GetKey(olc::Key::T).bReleased)
			setEnd(nMouseCellX, nMouseCellY);

		if (GetKey(olc::Key::SPACE).bReleased) {
			bCalculate = true;
		}

		if (GetKey(olc::Key::NP_ADD).bReleased)
			nMaxSteps = std::min(120, nMaxSteps + 1);

		if (GetKey(olc::Key::NP_SUB).bReleased)
			nMaxSteps = std::max(0, nMaxSteps - 1);

		// Drawing
		Clear(olc::BLACK);
		for (int x = 0; x < nAreaWidth; x++)
			for (int y = 0; y < nAreaHeight; y++)
			{
				cell cur = cBoard[xy(x, y)];

				olc::Pixel pixel = cur.isObstacle ? olc::GREY : olc::BLUE;
				FillRect({ x * nCellSize + (nBorderSize / 2),  y * nCellSize + (nBorderSize / 2) }, { nCellSize - nBorderSize, nCellSize - nBorderSize }, pixel);

				if (cur.isStart) {
					FillRect({ x * nCellSize + (nBorderSize / 2),  y * nCellSize + (nBorderSize / 2) }, { nCellSize - nBorderSize, nCellSize - nBorderSize }, olc::GREEN);
				}
				if (cur.isEnd) {
					FillRect({ x * nCellSize + (nBorderSize / 2),  y * nCellSize + (nBorderSize / 2) }, { nCellSize - nBorderSize, nCellSize - nBorderSize }, olc::RED);
				}

				if (x == nMouseCellX && y == nMouseCellY)
					DrawRect({ x * nCellSize, y * nCellSize }, { nCellSize - (nBorderSize / 2), nCellSize - (nBorderSize / 2) }, olc::YELLOW);

				//DrawString(x * nCellSize, y * nCellSize, std::to_string(nHeightMap[xy(x, y)]));
			}

		cell *last = nullptr;
		for (auto& cell : cPath) {
			if (last != nullptr)
				DrawLine({ (cell.x * nCellSize) + nCellSize / 2, (cell.y * nCellSize) + nCellSize / 2 }, { (last->x * nCellSize) + nCellSize / 2, (last->y * nCellSize) + nCellSize / 2 }, olc::YELLOW);

			FillCircle({ (cell.x * nCellSize) + nCellSize / 2, (cell.y * nCellSize) + nCellSize / 2 }, nCellSize / 4, olc::YELLOW);
			
			last = &cell;
		}

		// Finalize Frame
		nMouseCellXLastFrame = nMouseCellX;
		nMouseCellYLastFrame = nMouseCellY;

		return true;
	}

	void CalculatePath(bool limit = false) {
		// helper
		auto xy = [&](int x, int y) {return y * nAreaWidth + x; };

		bCalculate = false;
		cPath.clear();

		for (int x = 0; x < nAreaWidth; x++)
			for (int y = 0; y < nAreaHeight; y++)
				nHeightMap[xy(x, y)] = cBoard[xy(x, y)].isObstacle ? -1 : 0;

		std::list<std::tuple<int, int, int>> known_nodes;
		known_nodes.push_back({ nEndX, nEndY, 1 });

		int32_t step = 0;

		while (!known_nodes.empty() && (!limit || step++ < nMaxSteps))
		{

			std::list<std::tuple<int, int, int>> found_nodes;

			for (auto& c : known_nodes) {
				int x = std::get<0>(c);
				int y = std::get<1>(c);
				int d = std::get<2>(c);

				nHeightMap[xy(x, y)] = d;

				if ((x + 1) < nAreaWidth && nHeightMap[xy(x + 1, y)] == 0)
					found_nodes.push_back({ x + 1, y, d + 1 });

				if ((x - 1) >= 0 && nHeightMap[xy(x - 1, y)] == 0)
					found_nodes.push_back({ x - 1, y, d + 1 });

				if ((y + 1) < nAreaWidth && nHeightMap[xy(x, y + 1)] == 0)
					found_nodes.push_back({ x, y + 1, d + 1 });

				if ((y - 1) >= 0 && nHeightMap[xy(x, y - 1)] == 0)
					found_nodes.push_back({ x, y - 1, d + 1 });
			}

			found_nodes.sort([&](const std::tuple<int, int, int>& v1, const std::tuple<int, int, int>& v2)
			{
				return xy(std::get<0>(v1), std::get<1>(v1)) < xy(std::get<0>(v2), std::get<1>(v2));
			});

			found_nodes.unique([&](const std::tuple<int, int, int>& v1, const std::tuple<int, int, int>& v2)
			{
				return xy(std::get<0>(v1), std::get<1>(v1)) == xy(std::get<0>(v2), std::get<1>(v2));
			});

			known_nodes.clear();
			known_nodes.insert(known_nodes.begin(), found_nodes.begin(), found_nodes.end());
		}

		cPath.clear();

		if (nHeightMap[xy(nStartX, nStartY)] > 0)
		{
			// a path was found
			int x = nStartX;
			int y = nStartY;
			int last_height = INT_MAX;

			do {
				
				int next_x = x;
				int next_y = y;

				cPath.push_back(cBoard[xy(x, y)]);

				if (x == nEndX && y == nEndY)
					break;

				if ((x + 1) < nAreaWidth && 0 <= nHeightMap[xy(x + 1, y)] && nHeightMap[xy(x + 1, y)] < last_height)
				{
					last_height = nHeightMap[xy(x + 1, y)];
					next_x = x + 1;
					next_y = y;
				}

				if ((x - 1) >= 0 && 0 <= nHeightMap[xy(x - 1, y)] && nHeightMap[xy(x - 1, y)] < last_height)
				{
					last_height = nHeightMap[xy(x - 1, y)];
					next_x = x - 1;
					next_y = y;
				}

				if ((y + 1) < nAreaHeight && 0 <= nHeightMap[xy(x, y + 1)] && nHeightMap[xy(x, y + 1)] < last_height)
				{
					last_height = nHeightMap[xy(x, y + 1)];
					next_x = x;
					next_y = y + 1;
				}

				if ((y - 1) >= 0 && 0 <= nHeightMap[xy(x, y - 1)] && nHeightMap[xy(x, y - 1)] < last_height)
				{
					last_height = nHeightMap[xy(x, y - 1)];
					next_x = x;
					next_y = y - 1;
				}

				x = next_x;
				y = next_y;
			} while (last_height > 0);
		}

	}
};