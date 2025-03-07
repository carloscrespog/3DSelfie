#include <pcl/console/time.h>
#include <pcl/console/parse.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/io/auto_io.h>
#include <pcl/visualization/common/actor_map.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <vtkRenderWindow.h>

//Useful typedefs
typedef pcl::PointXYZRGBA PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

//Filthy globals
static PointCloudT::Ptr cld(new PointCloudT);
static pcl::PointCloud<pcl::Normal>::Ptr cld_normals (new pcl::PointCloud<pcl::Normal>);
static pcl::visualization::PCLVisualizer::Ptr viewer;

//Applications parameters
static float radius(0.1f);

int
visualize_normals()
{
	viewer.reset(new pcl::visualization::PCLVisualizer("HipsterTech Normal Check"));
  	viewer->setBackgroundColor (1, 1, 1);  	
	viewer->addPointCloud (cld);
	viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3);
	viewer->addPointCloudNormals<PointT, pcl::Normal> (cld, cld_normals, 1, 0.05, "normals");
	viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 0.0, 0.0, "normals"); 
     viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_LINE_WIDTH, 3, "normals"); 
	viewer->addCoordinateSystem (1.0);
	viewer->initCameraParameters ();
	viewer->setCameraPosition (0.0, -4.0, 0, 0.0, 0.0, 0.0,  0.0, 0.0, 1.0);
	return 0;
}

int
normal_estimation()
{
	pcl::console::TicToc time;
	pcl::NormalEstimationOMP<PointT, pcl::Normal> ne;
	pcl::search::KdTree<PointT>::Ptr tree (new pcl::search::KdTree<PointT> ());

	ne.setInputCloud (cld);
	ne.setSearchMethod (tree);
	ne.setRadiusSearch (radius);
	ne.setViewPoint(0.0f, -4.0f, 0.f);
	time.tic();
	ne.compute (*cld_normals);
	std::cout << "Took " << time.toc()/1000.f 
		<< "(s) to estimate normals" << std::endl
		<< "Used " << radius << " search radius." << endl;
	return 0;
}

int
parse_console_arguments(const int argc, char** const argv)
{
	//Not enough arguments
	if(argc < 2)
		return -1;

	//Print help and usage
	if(pcl::console::find_switch(argc, argv, "-h")
		|| pcl::console::find_switch(argc, argv, "--help"))
	{
		cout << "Usage: ht_normal_check <file> [options]" << endl
			<< "Options" << endl
			<< "-h,--help\t prints this help message" << endl
			<< "-r radius \t set the radius used for normal search" << endl;
		return 1;
	}

	//Load cloud
	if(pcl::io::load<PointT>(argv[1], *cld) < 0)
    {
        PCL_ERROR ("Error loading cloud %s.\n", argv[1]);
        return -1;
    }

    //Parse radius
    pcl::console::parse_argument(argc, argv, "-r",radius);

	return 0;
}

int
main(int argc, char **argv)
{
	int ret(0);

	/* Parse arguments */
    if((ret = parse_console_arguments(argc, argv)))
        return ret;

    /* Normal Estimation */
    normal_estimation();

    /* Visualize Normals */
    visualize_normals();

    /* VTK hook */
    vtkSmartPointer< vtkRenderWindow > win(viewer->getRenderWindow());	

    /* Main Loop */
	while (!viewer->wasStopped() && win->IsDrawable())
	{
		viewer->spinOnce (100);
		boost::this_thread::sleep (boost::posix_time::microseconds (100000));
	}
	return 0;
}