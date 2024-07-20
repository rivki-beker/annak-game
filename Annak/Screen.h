#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "Game.h"

class Screen {
public:
    Screen();
    void run();
private:
    void drawWorld();
    void display();
    void loadImages(std::vector<cv::Mat>& images);
    void loadObjectsImages();
    std::string ImgName(const std::string& name);
    void drawGrid(cv::Mat& tile, int gridSize);
    void drawObjects();
    void drawObject(const std::string& type, int x, int y, int width, int height);
    static void onMouseClickStatic(int event, int x, int y, int flags, void* userdata);
    void onMouseClick(int event, int x, int y, int flags);
    int pixelsForCell;
    int pixelsForTile;
    cv::Mat backgroundImage;
    Game game;
    Input input;
    std::unordered_map<std::string, cv::Mat> objectsImages;
    std::map<string,std::set<std::pair<int, int>>> drawnObjects;
};
