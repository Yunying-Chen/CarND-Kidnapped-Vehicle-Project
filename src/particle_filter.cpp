
#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"
#define EPS 0.0001
using namespace std;
static default_random_engine gen;
void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
	
	if(is_initialized) return ;
	num_particles = 100;
	
	double std_x, std_y, std_theta;
	std_x = std[0];
	std_y = std[1];
	std_theta = std[2];

	normal_distribution<double> dist_x(x, std_x);
	normal_distribution<double> dist_y(y, std_y);
	normal_distribution<double> dist_theta(theta, std_theta);
	
	for (int i=0;i<num_particles;i++)
	{
		Particle particle;
		particle.id = i;
		particle.x = dist_x(gen);
		particle.y = dist_y(gen);
		particle.theta=dist_theta(gen);
		particle.weight = 1.0;
		
		particles.push_back(particle);
		
	}
	is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
	double std_x = std_pos[0];
	double std_y = std_pos[1];
	double std_theta = std_pos[2];
	normal_distribution<double> dist_x(0, std_x);
	normal_distribution<double> dist_y(0, std_y);
	normal_distribution<double> dist_theta(0, std_theta);
  
	for(int i=0;i<num_particles;i++)
	{
		double theta = particles[i].theta;
		if (fabs(yaw_rate)<EPS)
		{
			particles[i].x += velocity * delta_t * cos( theta );
			particles[i].y += velocity * delta_t * sin( theta );
		}
		else
		{
			particles[i].x += (velocity * ( sin( theta + yaw_rate * delta_t )- sin(theta))) / yaw_rate;
			particles[i].y += (velocity * ( cos(theta) - cos(theta + yaw_rate*delta_t))) / yaw_rate;
			particles[i].theta += yaw_rate*delta_t;			
		}
		
		particles[i].x += dist_x(gen);
		particles[i].y += dist_y(gen);
		particles[i].theta += dist_theta(gen);
		
	}
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
	unsigned int n_predicted = predicted.size();
	unsigned int n_observations = observations.size();
	for (unsigned int i=0;i<n_observations;i++)
	{
		double min_distance = numeric_limits<double>::max();
		int mapID=-1;
		for (unsigned int j=0;j<n_predicted;j++)
		{
			double x_distance = observations[i].x - predicted[j].x;
			double y_distance = observations[i].y - predicted[j].y;
			
			double distance = x_distance*x_distance+y_distance*y_distance;
			
			if (distance<min_distance)
			{
				min_distance = distance;
				mapID = predicted[j].id;
				
			}
			
		}
		observations[i].id = mapID;
	}
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
	double stdX= std_landmark[0];
	double stdY = std_landmark[1];
	for (int i=0;i<num_particles;i++)
	{
		double x = particles[i].x;
		double y = particles[i].y;
		double theta = particles[i].theta;
		vector<LandmarkObs> predictions;
		for (unsigned int j=0;j<map_landmarks.landmark_list.size();j++)
		{
			float landmark_x = map_landmarks.landmark_list[j].x_f;
			float landmark_y = map_landmarks.landmark_list[j].y_f;
			int landmark_id = map_landmarks.landmark_list[j].id_i;
			double dx = x-landmark_x;
			double dy = y-landmark_y;
			if((dx*dx+dy*dy)<=(sensor_range*sensor_range))
				predictions.push_back(LandmarkObs{landmark_id,landmark_x,landmark_y});
		}
		
		vector<LandmarkObs> Map_transformed;
		for (unsigned int j=0;j<observations.size();j++)
		{
			double x_trans = observations[j].x*cos(theta) - observations[j].y*sin(theta)+x;
			double y_trans = observations[j].x*sin(theta) + observations[j].y*cos(theta)+y;
			Map_transformed.push_back(LandmarkObs{observations[j].id,x_trans,y_trans});
		}
		dataAssociation(predictions,Map_transformed);
		
		particles[i].weight=1.0;
		
		for (unsigned int j=0;j<Map_transformed.size();j++)
		{
			double observe_x = Map_transformed[j].x;
			double observe_y = Map_transformed[j].y;
			double pred_x,pred_y;
			
			unsigned int k=0;
			while ((k<predictions.size())&&(predictions[k].id != Map_transformed[j].id))
				k++;
			if(k<predictions.size())
			{
				pred_x = predictions[k].x;
				pred_y = predictions[k].y;
			}
			double d_x = observe_x - pred_x;
			double d_y= observe_y - pred_y;

			double weight = (1/(2*M_PI*stdX*stdY))*exp( -( pow(d_x,2)/(2*pow(stdX, 2)) + (pow(d_y,2)/(2*pow(stdY, 2)))));
			if (weight<EPS)
				particles[i].weight *= EPS;
			else 
				particles[i].weight *=weight;
		}
		
	}


}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
  vector<double> weights;
  double maxWeight = numeric_limits<double>::min();
  for(int i = 0; i < num_particles; i++) {
    weights.push_back(particles[i].weight);
    if ( particles[i].weight > maxWeight ) {
      maxWeight = particles[i].weight;
    }
  }

  // Creating distributions.
  uniform_real_distribution<double> distDouble(0.0, maxWeight);
  uniform_int_distribution<int> distInt(0, num_particles - 1);

  // Generating index.
  int index = distInt(gen);

  double beta = 0.0;

  // the wheel
  vector<Particle> resampledParticles;
  for(int i = 0; i < num_particles; i++) {
    beta += distDouble(gen) * 2.0;
    while( beta > weights[index]) {
      beta -= weights[index];
      index = (index + 1) % num_particles;
    }
    resampledParticles.push_back(particles[index]);
  }

  particles = resampledParticles;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates

    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
	return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
