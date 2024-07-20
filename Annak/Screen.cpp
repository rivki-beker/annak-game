#include "Screen.h"
#include <iostream>
#include <opencv2/opencv.hpp>

Screen::Screen() : input("", ""), game(input), pixelsForCell(30) {}

void Screen::run() {
	input.parse_and_store();
	game.world = World(input.world->data);
	game.handleStartCommands();
	loadObjectsImages();
	drawWorld();
	cv::namedWindow("Annak Game", cv::WINDOW_NORMAL);
	cv::setMouseCallback("Annak Game", onMouseClickStatic, this);
	cv::resizeWindow("Annak Game", backgroundImage.cols, backgroundImage.rows);
	while (true) {
		cv::imshow("Annak Game", backgroundImage);

		char key = cv::waitKey(10);
		if (key == 27)
			break;
	}

	cv::destroyAllWindows();
}

void Screen::onMouseClick(int event, int x, int y, int flags) {
	if (event == cv::EVENT_LBUTTONDOWN) {
		// Calculate tile coordinates based on mouse click
		int tileX = x / pixelsForTile;
		int tileY = y / pixelsForTile;

		// Calculate cell coordinates within the tile
		int cellX = (x % pixelsForTile) / pixelsForCell;
		int cellY = (y % pixelsForTile) / pixelsForCell;

		// Combine tile and cell coordinates to get the cell coordinates in the world
		int worldX = tileX * Tile::size + cellX + 1;  // +1 for 1-based index
		int worldY = tileY * Tile::size + cellY + 1;  // +1 for 1-based index

		game.handleSelectCommand(worldX, worldY);
	}
}

void Screen::onMouseClickStatic(int event, int x, int y, int flags, void* userdata) {
	Screen* screen = static_cast<Screen*>(userdata);
	if (screen) {
		screen->onMouseClick(event, x, y, flags);
	}
}

void Screen::loadImages(std::vector<cv::Mat>& images) {
	for (auto& entry : Configuration::config()["Tiles"].items()) {
		std::string imageName = ImgName(entry.key());
		images.emplace_back(cv::imread("./Tiles/" + imageName));
	}
}

void Screen::loadObjectsImages() {
	std::map<std::string, std::string> objectFiles = {
		{"People", "./People/people.png"},
		{"Village", "./Settlements/village.png"},
		{"City", "./Settlements/city.png"},
		{"Car", "./Vehicles/car.png"},
		{"Truck", "./Vehicles/truck.png"},
		{"Helicopter", "./Vehicles/helicopter.png"},
		{"Road", "./Tiles/tile_road.png"}
	};

	for (const auto& file : objectFiles) {
		cv::Mat image = cv::imread(file.second);
		if (image.empty()) {
			std::cerr << "Failed to load image: " << file.second << std::endl;
			continue;
		}
		objectsImages[file.first] = image;
	}
}

std::string Screen::ImgName(const std::string& name) {
	std::string formattedName = "tile";

	for (char c : name) {
		if (isupper(c)) {
			formattedName += '_';
			formattedName += tolower(c);
		}
		else {
			formattedName += c;
		}
	}

	return formattedName + ".png";
}

void Screen::drawGrid(cv::Mat& tile, int gridSize) {
	cv::Scalar gridColor = cv::Scalar(0, 0, 0); // Black color for the grid
	int tileWidth = tile.cols;
	int tileHeight = tile.rows;
	int cellSize = tileWidth / gridSize;

	for (int y = cellSize; y < tileHeight; y += cellSize) {
		cv::line(tile, cv::Point(0, y), cv::Point(tileWidth, y), gridColor, 1);
	}
	for (int x = cellSize; x < tileWidth; x += cellSize) {
		cv::line(tile, cv::Point(x, 0), cv::Point(x, tileHeight), gridColor, 1);
	}
}

void Screen::drawWorld() {
	std::vector<cv::Mat> images;
	loadImages(images);
	int borderSize = 1;  // Size of the border between tiles

	pixelsForTile = Tile::size * pixelsForCell + borderSize;
	int numCols = input.world->data[0].size();
	int numRows = input.world->data.size();

	backgroundImage = cv::Mat(numRows * pixelsForTile - borderSize,
		numCols * pixelsForTile - borderSize,
		CV_8UC3, cv::Scalar(68, 8, 127)); // White background

	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			int tileIndex = std::stoi(input.world->data[i][j]) - 1;
			cv::Mat tile = images[tileIndex];
			if (tile.empty()) {
				std::cerr << "Missing tile image for index " << tileIndex << std::endl;
				continue;
			}

			cv::Mat largeTileImage;
			cv::resize(tile, largeTileImage, cv::Size(pixelsForTile - borderSize, pixelsForTile - borderSize));
			drawGrid(largeTileImage, Tile::size);

			int x = j * pixelsForTile;
			int y = i * pixelsForTile;

			cv::Rect roi(cv::Point(x, y), largeTileImage.size());
			largeTileImage.copyTo(backgroundImage(roi));

			// Draw the border around the tile
			cv::rectangle(backgroundImage, roi, cv::Scalar(68, 8, 127), borderSize);
		}
	}

	// Draw the objects on the world
	drawObjects();
}


void Screen::drawObjects() {
	drawnObjects.clear();
	int worldHeight = input.world->data.size();
	int worldWidth = worldHeight > 0 ? input.world->data[0].size() : 0;

	for (int y = 1; y <= worldHeight * Tile::size; ++y) {
		for (int x = 1; x <= worldWidth * Tile::size; ++x) {
			auto& cell = game.world.getCell(x, y);
			if (!cell.empty()) {
				auto& component = cell.begin()->second;
				std::string type = component->category();
				int componentSize = component->size();

				bool alreadyDrawn = false;
				for (int dy = 0; dy < componentSize; ++dy) {
					for (int dx = 0; dx < componentSize; ++dx) {
						if (drawnObjects[type].find({ x - dx, y - dy }) != drawnObjects[type].end()) {
							alreadyDrawn = true;
							break;
						}
					}
					if (alreadyDrawn) break;
				}

				if (!alreadyDrawn) {
					drawObject(type, x - 1, y - 1, componentSize, componentSize);

					drawnObjects[type].insert({ x, y });
				}
			}
		}
	}
}

void Screen::drawObject(const std::string& type, int x, int y, int width, int height) {
	if (objectsImages.find(type) != objectsImages.end()) {
		cv::Mat objectImage = objectsImages[type];

		// Calculate size based on pixels for cell
		int pixelWidth = width / Tile::size * pixelsForTile + (width % Tile::size) * pixelsForCell;
		int pixelHeight = height / Tile::size * pixelsForTile + (height % Tile::size) * pixelsForCell;

		cv::resize(objectImage, objectImage, cv::Size(pixelWidth, pixelHeight));

		// Calculate position based on pixels for tile and consider border size
		int posX = x / Tile::size * pixelsForTile + x % Tile::size * pixelsForCell;
		int posY = y / Tile::size * pixelsForTile + y % Tile::size * pixelsForCell;

		cv::Rect roi(cv::Point(posX, posY), objectImage.size());
		objectImage.copyTo(backgroundImage(roi));
	}
	else {
		std::cerr << "Unknown object type: " << type << std::endl;
	}
}

void Screen::display() {
	//--------------------------------------
}