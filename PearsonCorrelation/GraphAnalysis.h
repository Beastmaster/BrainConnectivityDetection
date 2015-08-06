/**
Author:     Qin Shuo
Site:       Huazhong University of S & T, Wuhan
Create Date:2015.5.31

Function:
		Analysis of correlation matrix based on graph theory
**/

#ifndef _GRAPH_ANALYSIS_H_
#define _GRAPH_ANALYSIS_H_

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string>

class graph_node
{
public:
	graph_node();
	~graph_node();
};

class graph_analysis
{
public:
	typedef std::pair<std::string, std::vector<double> > row_type2;
	typedef std::vector<row_type2> col_type2;

	graph_analysis();
	~graph_analysis();


	int compute_Degree(int i); //compute i degree of i_th node
	int compute_Distance(int a,int b);//distance between a and b
	int compute_Trangles(int i);//Number of triangles around a node i
	int compute_Integration();//Characteristic path length of the network
	double compute_Global_Efficiency();//Global efficiency of the network
	double compute_Cluster_Coefficient();//Measures of segregation	Clustering coefficient
	double compute_Transitivity();//Transitivity of the network
	double compute_Local_Efficiency();//Local efficiency of the network
	double compute_Modularity();//
	double compute_Closeness_Centrality();//
	double compute_Betweenness_Centrality();
	double compute_Z_Score();
	double compute_Participation_Coefficient();
	double compute_Network_Motifs();
	double compute_Motif_Z_Score();
	double compute_Motif_Fingerprint();
	double compute_Degree_Distribution();
	double compute_Average_Neighbor_Degree();


private:

};




#endif

