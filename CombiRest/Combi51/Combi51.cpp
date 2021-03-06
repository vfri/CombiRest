// Combi51.cpp: определяет точку входа для консольного приложения.
// Нахождение макисмального паросочетания (полного паросочетания)

#include "stdafx.h"

static const int MAXV = 101; // максимальное число вершин (индекс 0 -- нерабочий)

struct Graph
{
	size_t part1num;
	size_t part2num;
	size_t num1Vert[MAXV];
	std::vector<int> neighs1[MAXV];
	char mark1[MAXV];
	int matches[MAXV];
};

void ReadGraph(const std::string& graphFileName, Graph& graph)
{
	std::ifstream graphFile(graphFileName);

	std::string line;
	std::getline(graphFile, line);
	std::cout << line << std::endl;
	std::stringstream ss;
	ss << line;
	
	size_t part1num = 0;
	size_t part2num = 0;
	ss >> part1num >> part2num;
	graph.part1num = part1num;
	graph.part2num = part2num;
	if (part1num + 1 > MAXV) graph.part1num = MAXV - 1;
	if (part2num + 1 > MAXV) graph.part2num = MAXV - 1;
	std::cout << graph.part1num << " in one part and " << graph.part2num << " in other" << std::endl;

	for (size_t i = 1; i <= part1num; ++i)
	{
		graph.mark1[i] = 'n';
		graph.neighs1[i].assign(part2num + 1, 1);
	}
	for (size_t i = 1; i <= part2num; ++i)
	{
		graph.matches[i] = -1;
	}
		
	size_t part1vert = 1;
	while (std::getline(graphFile, line))
	{
		std::cout << line << std::endl;
		std::stringstream ss;
		ss << line;
		int edge;
		size_t toVert = 1;
		graph.num1Vert[part1vert] = 0;
		int count = 0;
		while (ss >> edge)
		{
			if (edge)
			{
				graph.neighs1[part1vert][count] = toVert;
				(graph.num1Vert[part1vert])++;
				count++;
			}
			toVert++;
		}
		part1vert++;
	}
	std::cout << "Read graph done!" << std::endl;
}

void PrintTwopartGraph(Graph const& graph)
{
	std::cout << "++++++++++++++++++++++++++++++++++++++++++\n";
	std::cout << "Graph on " << graph.part1num << " in first part and " << 
		graph.part2num << " in second part" << std::endl;
	for (size_t i = 1; i <= graph.part1num; ++i)
	{
		std::cout << "Vertex [" << i << "] of degree " << graph.num1Vert[i] << ". Neighbours are: ";
		for (size_t j = 0; j < graph.num1Vert[i]; ++j)
		{
			std::cout << "[" << graph.neighs1[i][j] << "] ";
		}
		std::cout << "\n"; 
		std::cout << "Vertex marked " << graph.mark1[i] << std::endl;
	}
	std::cout << "\nMatches in twopart graph:\n";
	
	int counter = 0;
	for (size_t i = 1; i <= graph.part2num; ++i)
	{
		if (graph.matches[i] != -1)
		{
			std::cout << "[" << graph.matches[i] << "] -- [" << i << "]\n";
			counter++;
		}
	}
	std::cout << counter << " pairs overall.\n";
	if (counter == std::min(graph.part1num, graph.part2num)) std::cout << "Matching is full\n";


	std::cout << "\n++++++++++++++++++++++++++++++++++++++++++\n" << std::endl;
}



bool AlgKuhn(Graph& graph, size_t vert) // нахождение увелчивающей цепи
{
	
	if (graph.mark1[vert] == 'y') return false;
	
	graph.mark1[vert] = 'y';
	for (size_t i = 0; i < graph.num1Vert[vert]; ++i) 
	{
		int p2nei = graph.neighs1[vert][i];
		if (graph.matches[p2nei] == -1 || AlgKuhn(graph, graph.matches[p2nei]))
		{
			graph.matches[p2nei] = vert;
			return true;
		}
	}
	
	return false;
}


void Kuhn4AlltheGraph(Graph& graph)
{
	for (size_t vert = 1; vert <= graph.part1num; ++vert) 
	{
		for (size_t i = 1; i <= graph.part1num; ++i) graph.mark1[i] = 'n';
		AlgKuhn(graph, vert);
	}
}



int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Graph file expected!" << std::endl;
		return 1;
	}
	else
	{
		std::string graphFileName(argv[1]);
		std::cout << graphFileName << std::endl;
		Graph graph;
		ReadGraph(graphFileName, graph);
		
		PrintTwopartGraph(graph);

		Kuhn4AlltheGraph(graph);

		std::cout << "The Kuhn algorithm result:\n" << std::endl;
		PrintTwopartGraph(graph);

	}
	
	return 0;
}

