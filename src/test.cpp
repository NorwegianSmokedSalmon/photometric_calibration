#include "StandardIncludes.h"

#include "ImageReader.h"
#include "Tracker.h"
#include "RapidExposureTimeEstimator.h"
#include "Database.h"
#include "NonlinearOptimizer.h"
#include "CLI11.hpp"

using namespace std;

struct Settings{
    int start_image_index;      // Start image index.
    int end_image_index;        // End image index.
    int image_width;            // Image width to resize to.
    int image_height;           // Image height to resize to.
    int visualize_cnt;          // Visualize every visualize_cnt image (tracking + correction), rather slow.
    int tracker_patch_size;     // Image patch size used in tracker.
    int nr_pyramid_levels;      // Number of image pyramid levels used in tracker.
    int nr_active_features;     // Number of features maintained for each frame.
    int nr_images_rapid_exp;    // Number of images for rapid exposure time estimation.
    int nr_active_frames;       // Number of frames maintained in database.
    int keyframe_spacing;       // Spacing for sampling keyframes in backend optimization.
    int min_keyframes_valid;    // Minimum amount of keyframes a feature should be present to be included in optimization.
    string image_folder;        // Image folder.
    string exposure_gt_file;    // Exposure times ground truth file.
    string calibration_mode;    // Choose "online" or "batch".
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./test_gradient_rgb <image_path>" << std::endl;
        return -1;
    }

    cv::Mat raw_image = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (raw_image.empty())
    {
        std::cerr << "Failed to load image." << std::endl;
        return -1;
    }

    // ===== ✅ 将RGB图像转为灰度图并保存到同一目录 =====
    cv::Mat gray_image;
    cv::cvtColor(raw_image, gray_image, cv::COLOR_BGR2GRAY);

    // 构造保存路径（和原图同目录）
    std::string image_path(argv[1]);
    size_t last_slash = image_path.find_last_of("/\\");
    std::string folder = (last_slash == std::string::npos) ? "." : image_path.substr(0, last_slash);
    std::string filename = image_path.substr(last_slash + 1);
    std::string gray_filename = "gray_" + filename;

    std::string gray_output_path = folder + "/" + gray_filename;

    // 保存灰度图
    if (!cv::imwrite(gray_output_path, gray_image))
    {
        std::cerr << "Failed to save grayscale image to: " << gray_output_path << std::endl;
    }
    else
    {
        std::cout << "Saved grayscale image to: " << gray_output_path << std::endl;
    }

    // ✅ 用栈变量安全初始化 Settings
    Settings run_settings;
    run_settings.image_width = raw_image.cols;
    run_settings.image_height = raw_image.rows;
    run_settings.tracker_patch_size = 2;
    run_settings.nr_active_features = 150;
    run_settings.nr_pyramid_levels = 3;

    // ✅ 初始化其他对象（不使用 image_reader）
    Database database(run_settings.image_width, run_settings.image_height);
    Tracker tracker(run_settings.tracker_patch_size,
                    run_settings.nr_active_features,
                    run_settings.nr_pyramid_levels,
                    &database);

    // 灰度图计算
    cv::cvtColor(raw_image, gray_image, cv::COLOR_BGR2GRAY);

    // 灰度梯度图（用 computeGradientImageRGB 单通道版本）
    cv::Mat gray_gradient;
    tracker.computeGradientImage(gray_image, gray_gradient); // 会处理单通道

    // RGB梯度图（已有）
    cv::Mat rgb_gradient;
    tracker.computeGradientImageRGB(raw_image, rgb_gradient);

    // 调用提取函数（假设内部 m_database 记录的是 frame）
    tracker.initialFeatureExtraction(gray_image, gray_gradient, 1.0);     // 用于灰度图
    tracker.initialFeatureExtractionRGB(raw_image, rgb_gradient, 1.0);    // 用于RGB图

    // 获取两个 Frame 中的特征点（m_database 最后两帧分别是这两个）
    const Frame& gray_frame = tracker.getDatabase()->m_tracked_frames[tracker.getDatabase()->m_tracked_frames.size() - 2];
    const Frame& rgb_frame  = tracker.getDatabase()->m_tracked_frames[tracker.getDatabase()->m_tracked_frames.size() - 1];

    // 可视化灰度特征点
    cv::Mat vis_gray;
    cv::cvtColor(gray_frame.m_image, vis_gray, cv::COLOR_GRAY2BGR); // 转为三通道用于画点
    for (const auto& f : gray_frame.m_features)
    {
        cv::circle(vis_gray, f->m_xy_location, 2, cv::Scalar(0, 255, 0), -1);
    }

    // 可视化RGB特征点
    cv::Mat vis_rgb = rgb_frame.m_image.clone();
    for (const auto& f : rgb_frame.m_features)
    {
        cv::circle(vis_rgb, f->m_xy_location, 2, cv::Scalar(0, 0, 255), -1);
    }
    while (true)
    {
        cv::Mat input_image = raw_image.clone();
        cv::Mat gradient_image_RGB;
        cv::Mat gradient_image_gray;

        tracker.computeGradientImageRGB(input_image, gradient_image_RGB);
        std::cout << "1111111111111111111" << std::endl;
        tracker.computeGradientImage(gray_image, gradient_image_gray);
        std::cout << "2222222222222222222" << std::endl;
        // 可视化
        cv::imshow("Gray Image + Features", vis_gray);
        cv::imshow("RGB Image + Features", vis_rgb);
        cv::imshow("RGB Gradient Image", gradient_image_RGB);
        cv::imshow("Grayscale Gradient Image", gradient_image_gray);

        // 每 30ms 刷新一次，按 ESC 退出
        int key = cv::waitKey(30);
        if (key == 27) break; // ESC
    }
    return 0;
}

