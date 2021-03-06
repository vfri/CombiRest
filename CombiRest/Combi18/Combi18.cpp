// Combi18.cpp: определяет точку входа для консольного приложения.
// Перебор вариантов в задаче коммивояжера с отсеиванием ветвей поиска оцениванием возможной стоимости 
// гамильтонова цикла.
// Граф задается списком смежности. В первой строке -- число вершин пробел число ребер,
// В последующих строках -- один конец ребра пробел другой конец ребра пробел стоимость

#include "stdafx.h"

static const size_t MAXV = 11;
static const int INFT = 1000000000;

struct Edge
{
	int vert1;
	int vert2;
	int weight;
	char used;

	Edge(int v1, int v2, int w)
		:vert1(v1), vert2(v2), weight(w), used('n')
	{}
};

struct Graph
{
	size_t numVert = 0;
	size_t numNeigh[MAXV];
	std::vector<int> neighbours[MAXV];  // соседи данной вершины
	std::vector<int> weight[MAXV];		// веса соседей
	std::vector<Edge> allEdges;			// список всех ребер
};

void ReadGraph(const std::string& graphFileName, Graph& graph)
{
	std::ifstream graphFile(graphFileName);
	size_t numEdges = 0;
	size_t numVert = 0;
	graphFile >> numVert >> numEdges;
	graph.numVert = numVert;
	std::cout << graph.numVert << " vertices + " << numEdges << " edges" << std::endl;

	for (size_t i = 1; i <= numVert; ++i) graph.numNeigh[i] = 0;

	graph.allEdges.clear();

	for (size_t i = 1; i <= numEdges; ++i)
	{
		int vert1, vert2, weight;
		graphFile >> vert1 >> vert2 >> weight;
		graph.neighbours[vert1].push_back(vert2);
		graph.neighbours[vert2].push_back(vert1);
		graph.weight[vert1].push_back(weight);
		graph.weight[vert2].push_back(weight);
		graph.numNeigh[vert1]++;
		graph.numNeigh[vert2]++;
		Edge nextEdge(vert1, vert2, weight);
		graph.allEdges.push_back(nextEdge);
	}
	std::cout << "Read graph done!" << std::endl;
}

void PrintEdge(Edge const& edge)
{
	std::cout << "[" << edge.vert1 << "]--[" << edge.vert2 << "] (" << edge.weight << ")\n";
}

void PrintGraph(const Graph& graph)
{
	std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++++++++\n";
	std::cout << "Graph on " << graph.numVert << " vertices." << std::endl;
	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		std::cout << "Vertex [" << i << "] of degree " << graph.numNeigh[i] << ". Neighbours are: ";
		for (size_t j = 0; j < graph.numNeigh[i]; ++j)
		{
			std::cout << "[" << graph.neighbours[i][j] << "]" << "(" << graph.weight[i][j] << ") ";
		}
		std::cout << std::endl;
	}
	std::cout << "Edges list:\n";
	for (size_t i = 0; i < graph.allEdges.size(); ++i)
	{
		std::cout << i << ") ";
		PrintEdge(graph.allEdges[i]);
	}
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PrintEdgeSet(std::vector<Edge> const& eVec)
{
	std::cout << "Edge set:" << std::endl;
	for (auto& edge : eVec) PrintEdge(edge);
	std::cout << std::endl;
}

void SetEvecMarksN(std::vector<Edge>& eVec)
{
	for (auto& edge : eVec) edge.used = 'n';
}

void DelEdgeFromVert(std::vector<Edge>& eVec, int vert, std::vector<int>& degs)
{
	for (size_t i = 0; i < eVec.size(); ++i) 
	{
		size_t v1 = eVec[i].vert1;
		size_t v2 = eVec[i].vert2;
		if (((v1 == vert) || (v2 == vert)) && (eVec[i].used == 'n'))
		{
			degs[v1]--;
			degs[v2]--;
			eVec[i].used = 'y';
			break;
		}
	}
}

void EraseNonСyclic(std::vector<Edge>& eVec, std::vector<int>& degs) // все степени -- 0, 1 или 2. Убираем нециклические компоненты
{
	auto it = find(degs.begin() + 1, degs.end(), 1);
	while (it != degs.end())
	{
		DelEdgeFromVert(eVec, std::distance(degs.begin(), it), degs);
		it = find(degs.begin() + 1, degs.end(), 1);
	}
	SetEvecMarksN(eVec);
}

bool IsHamiltonLike(std::vector<Edge>& eVec, std::vector<int>& degs)
{
	DelEdgeFromVert(eVec, 1, degs);
	EraseNonСyclic(eVec, degs);
	
	int cycVertNum = std::count(degs.begin() + 1, degs.end(), 2);
	if (cycVertNum == 0) std::cout << "Hamilton find!\n";
	SetEvecMarksN(eVec);
	return cycVertNum == 0;
}

void FillEdgeSetDegrees(std::vector<Edge> const& eVec, std::vector<int>& degs, Graph const& graph)
{
	degs.resize(graph.numVert + 1);
	for (auto& edge : eVec)
	{
		degs[edge.vert1]++;
		degs[edge.vert2]++;
	}
}


bool IsEdgeSetAcceptable(std::vector<Edge>& eVec, Graph const& graph, bool& hamilton)
{
	size_t edNum = eVec.size();
	if (edNum > graph.numVert) return false; // множество ребер недопустимо, если ребер больше, чем вершин графа
	
	std::vector<int> eSetDegs;
	FillEdgeSetDegrees(eVec, eSetDegs, graph); // сейчас eSetDegs содержит степени вершин по подмножеству ребер eVec

	auto maxit = std::max_element(eSetDegs.begin() + 1, eSetDegs.end());
	if (*maxit > 2)
	{
		return false;				// или если некоторые вершины имеют степень больше 2
	}
	else
	{
		EraseNonСyclic(eVec, eSetDegs);
		size_t cycVertNum = std::count(eSetDegs.begin() + 1, eSetDegs.end(), 2);
		if ((cycVertNum > 0) && (cycVertNum < graph.numVert)) return false;	// или если образуются циклы дины меньше numVert
		if ((cycVertNum == graph.numVert) && IsHamiltonLike(eVec, eSetDegs)) hamilton = true;

	}
	std::cout << "\nAcceptable set of edges found:\n";
	PrintEdgeSet(eVec);
	
	return true;
}

int EdgeSetCost(std::vector<Edge> const& eVec)
{
	int sum = 0;
	for (auto& edge : eVec) sum += edge.weight;
	return sum;
}

void WeightsOfEdgesOnVertWithBigNums(Graph const& graph, std::vector<int>& bigNumEdgesWeights, int numBoard, int vert)
{	// ребра после numBoard-го по счету, инцидентные вершине vert, складываем в bigNumEdges (по возрастанию веса)
	
	for (size_t i = numBoard; i < graph.allEdges.size(); ++i)
	{
		if ((graph.allEdges[i].vert1 == vert) || (graph.allEdges[i].vert2 == vert))
		{
			bigNumEdgesWeights.push_back(graph.allEdges[i].weight);
		}
	}
	std::sort(bigNumEdgesWeights.begin(), bigNumEdgesWeights.end());
	
}

int EstimateOnEdgeSet(size_t viewed, std::vector<Edge> eVec, Graph const& graph)
{	// просмотрены первые viewed ребер графа; выбраны ребра eVec; строим нижнюю оценку стоимости гамильтонова цикла
	// на основе множества eVec
	int esti = EdgeSetCost(eVec);  // общая стоимость ребер множества eVec

	std::vector<int> degsOnViewed;
	FillEdgeSetDegrees(eVec, degsOnViewed, graph);
	
	std::vector<int> bigNumEdgesWeights;

	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		size_t numAddEdges = 2 - degsOnViewed[i];
		
		bigNumEdgesWeights.clear();
		WeightsOfEdgesOnVertWithBigNums(graph, bigNumEdgesWeights, viewed, i);
		
		if (bigNumEdgesWeights.size() >= numAddEdges)
		{
			for (size_t j = 0; j < numAddEdges; ++j) esti += bigNumEdgesWeights[j];
		}
		else
		{
			esti = INFT;
		}
	}
	
	return esti / 2;
}


void FindMinHamiltonCycle(Graph const& graph, size_t numEdgesViewed, std::vector<Edge>& eVec, 
	std::vector<Edge>& hCycle, int& minCost)
{
	std::cout << "\n************************\n";
	std::cout << "numEdgeViewed = " << numEdgesViewed << "\n";
	
	PrintEdgeSet(eVec);
	size_t numEdges = graph.allEdges.size();
	if ((numEdgesViewed + 1 < numEdges) && (eVec.size() + numEdges - 1 - numEdgesViewed >= graph.numVert))
	{
		FindMinHamiltonCycle(graph, numEdgesViewed + 1, eVec, hCycle, minCost); // случай, когда ребро с номером numEdgesViewed
	}	// не выбирается для цикла
		
	eVec.push_back(graph.allEdges[numEdgesViewed]);
	
	bool hamilton = false;
	if (IsEdgeSetAcceptable(eVec, graph, hamilton))
	{
		if (hamilton)
		{
			int cost = EdgeSetCost(eVec);
			if (cost < minCost)
			{
				hCycle = eVec;
				minCost = cost;
				std::cout << "\nMin changed!!!!!!!!!!!!!\n";
			}
		}
		else
		{
			int estimate = EstimateOnEdgeSet(numEdgesViewed + 1, eVec, graph);
			if ((numEdgesViewed + 1 < numEdges) && (eVec.size() + numEdges - numEdgesViewed >= graph.numVert)
				&& (estimate < minCost))
			{
				FindMinHamiltonCycle(graph, numEdgesViewed + 1, eVec, hCycle, minCost);
			}
		}
	}
	eVec.pop_back();
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
		PrintGraph(graph);
		std::vector<Edge> eVec;
		std::vector<Edge> hCycle;
		int minCost = EdgeSetCost(graph.allEdges);

		FindMinHamiltonCycle(graph, 0, eVec, hCycle, minCost);
		std::cout << "Final Hamilton cycle:\n";
		PrintEdgeSet(hCycle);
		std::cout << "Hamilton cycle cost is " << EdgeSetCost(hCycle) << std::endl;
	}
	return 0;
}
