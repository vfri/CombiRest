// Combi23.cpp: определяет точку входа для консольного приложения.
// Проверка планарности графа с помощью гамма-алгоритма. Предположения о графе: связный граф; 
// содержит хотя бы один цикл; не имеет мостов (в частности, висячих вершин)
// таким образом, любая вершина степени хотя бы 2
// Граф в файле. Первая строка -- число вершин пробел число ребер
// В остальных строках ребра: одна вершина пробел другая вершина
// Результат -- грани в виде наборов их вершин, в скобках -- номер шага, на котором сформировалась грань
// либо сообщение, что граф неплоский

#include "stdafx.h"

static const size_t MAXV = 101; // максимальное число вершин

struct Graph
{
	size_t numVert = 0;
	size_t numNeigh[MAXV];				// номера вершин начинаются с 1 (в numNeigh[0] лежит мусор)
	char inComp[MAXV];					// 'f' -- вершина грани, 'n' -- не участвует в компоненте, 
										// 't' -- в заполняемой компоненте, 'c' -- если неконтактная вершина сформированной компоненты
	int broadMark[MAXV];				// метка поиска в ширину. Отсутствие метки -- значение "-1"
	std::vector<int> neighbours[MAXV];  // соседи данной вершины
};

struct CompVert	
{
	int name;				// название вершины (совпадает с названием в графе -- целый положительный номер)
	bool contact = false;	// контактная или нет
			
	CompVert(int n, bool co)
		:name(n), contact(co)
	{}

};

struct Componenta					// компонента -- набор вершин, инцидентных еще не уложенным ребрам
{
	int startVert;					// вершина, на основе которой строилась компонента поиском в ширину
	std::vector<CompVert> vers;		// вершины
	std::vector<int> path;			// путь между контактными вершинами
	std::vector<int> facs;			// перечень граней, в которых лежит компонента
};

struct Face						// грань в плоской укладке
{
	int step;					// шаг, на котором сформирована грань
	std::vector<int> vrtx;		// набор вершин грани
};

void ReadGraph(const std::string& graphFileName, Graph& graph)
{
	std::ifstream graphFile(graphFileName);
	size_t numEdges = 0;
	size_t numVert = 0;
	graphFile >> numVert >> numEdges;
	graph.numVert = numVert;
	std::cout << graph.numVert << " vertices and " << numEdges << " edges" << std::endl;

	for (size_t i = 1; i <= numVert; ++i)
	{
		graph.numNeigh[i] = 0;
		graph.inComp[i] = 'n';
		graph.broadMark[i] = -1;
	}

	for (size_t i = 1; i <= numEdges; ++i)
	{
		int vert1, vert2;
		graphFile >> vert1 >> vert2;
		graph.neighbours[vert1].push_back(vert2);
		graph.neighbours[vert2].push_back(vert1);
		graph.numNeigh[vert1]++;
		graph.numNeigh[vert2]++;
	}
	std::cout << "Read graph done!" << std::endl;
}

void ClearBroadMarks(Graph& graph)
{
	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		graph.broadMark[i] = -1;
	}
}

void ClearInCompMarks(Graph& graph)
{
	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		if (graph.inComp[i] != 'f') graph.inComp[i] = 'n';
	}
}


void PrintGraph(Graph const& graph)
{
	std::cout << "++++++++++++++++++++++++++++++++++++++++++\n";
	std::cout << "Graph on " << graph.numVert << " vertices." << std::endl;
	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		std::cout << "Vertex [" << i << "] of degree " << graph.numNeigh[i] << ". Neighbours are: ";
		for (size_t j = 0; j < graph.numNeigh[i]; ++j)
		{
			std::cout << "[" << graph.neighbours[i][j] << "] ";
		}
		std::cout << std::endl;
		std::cout << "inComp is " << graph.inComp[i];
		std::cout << "\n" << std::endl;
	}
	std::cout << "++++++++++++++++++++++++++++++++++++++++++\n" << std::endl;
}

void PrintComponenta(Componenta const& comp)
{
	std::cout << "\nComponenta vertices: ";
	for (auto& vert : comp.vers) std::cout << vert.name << " ";
	std::cout << "\npath: ";
	for (auto& num : comp.path) std::cout << num << " ";
	std::cout << std::endl;
}

void PrintFaces(std::vector<Face> const& faceSet)
{
	std::cout << "\nFaces:" << std::endl;
	for (size_t i = 0; i < faceSet.size(); ++i)
	{
		std::cout << "(" << faceSet[i].step << "): ";
		std::copy(faceSet[i].vrtx.begin(), faceSet[i].vrtx.end(), std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;
	}
	std::cout << std::endl;
}


void FindAnyCycle(Graph const& graph, std::vector<int>& cycle) // находит в графе цикл (из трех и более вершин)
{
	cycle.clear();
	int currVert = 1;
	cycle.push_back(currVert);
	bool newVert = true;

	while (newVert)
	{
		newVert = false;
		for (auto& apprNeigh : graph.neighbours[currVert])
		{
			if (find(cycle.begin(), cycle.end(), apprNeigh) == cycle.end())
			{
				newVert = true;
				currVert = apprNeigh;
				cycle.push_back(currVert);
				break;
			}
		}
	}
	auto cycRealBegin = find_first_of(cycle.begin(), cycle.end(),
		graph.neighbours[currVert].begin(), graph.neighbours[currVert].end());
	cycle.erase(cycle.begin(), cycRealBegin);
}

void InitFaceSet(Graph& graph, std::vector<Face>& faceSet)
{
	Face init1;
	FindAnyCycle(graph, init1.vrtx);
	init1.step = 0;
	faceSet.push_back(init1);
	std::vector<int> revCycle(init1.vrtx.size());
	std::reverse_copy(std::begin(init1.vrtx), std::end(init1.vrtx), std::begin(revCycle));
	Face init2;
	init2.vrtx = revCycle;
	init2.step = 0;
	faceSet.push_back(init2);
	std::cout << "Initial faces:" << std::endl;
	PrintFaces(faceSet);
	for (auto& vert : faceSet[0].vrtx) graph.inComp[vert] = 'f';
}

bool IsEdgeAFaceSide(int vert1, int vert2, Face const& face)
{
	auto it1 = find(face.vrtx.begin(), face.vrtx.end(), vert1);
	auto it2 = find(face.vrtx.begin(), face.vrtx.end(), vert2);
	if ((it1 == face.vrtx.end()) || (it2 == face.vrtx.end())) return false;
	int dnce = std::distance(it1, it2);
	return (std::abs(dnce) == 1) || (std::abs(dnce) == face.vrtx.size() - 1);
}

bool EdgeInGraph(Graph const& graph, int i, int j)
{
	return (find(graph.neighbours[i].begin(), graph.neighbours[i].end(), j) != graph.neighbours[i].end());
}


int FindMultiEdgeVertx(Graph& graph, int compStartVert) // находим вершины многореберной компоненты и начало пути в ней
{																	   // faceNeigh - FindMultiEdgeVertx
	int currMark = 0;
	graph.broadMark[compStartVert] = currMark;
	graph.inComp[compStartVert] = 't';

	
	int stepIntoFace = compStartVert;
	int faceNeigh;
	bool newMarked = true;
	while (newMarked)
	{
		newMarked = false;
		for (size_t i = 1; i <= graph.numVert; ++i)
		{
			if ((graph.broadMark[i] == currMark) && (graph.inComp[i] == 't'))
			{
				for (size_t j = 0; j < graph.numNeigh[i]; ++j)
				{
					int nei = graph.neighbours[i][j];
					if (graph.broadMark[nei] == -1)
					{
						graph.broadMark[nei] = currMark + 1;
						if (graph.inComp[nei] == 'n')
						{
							stepIntoFace = nei;
							graph.inComp[nei] = 't';
						}
						newMarked = true;
					}
				}
			}
		}
		currMark++;
	}

	
	
	for (auto& vert : graph.neighbours[stepIntoFace])
	{
		if (graph.inComp[vert] == 'f')
		{
			faceNeigh = vert;
			break;
		}
	}
	
	return faceNeigh;
}


bool IsNewVertInPath(std::vector<int> const& path, int vert)
{
	return find(path.begin(), path.end(), vert) == path.end();
}


void FindPathDepth(Graph& graph, int currVert, std::vector<int>& path, bool& isFound)
{
	if (graph.inComp[currVert] == 'f')
	{
		isFound = true;
	}
	if (graph.inComp[currVert] == 't')
	{
		for (size_t i = 0; i < graph.neighbours[currVert].size(); ++i)
		{
			int nei = graph.neighbours[currVert][i];
			if (IsNewVertInPath(path, nei) || (path[0] == nei))
			{
				path.push_back(nei);
				FindPathDepth(graph, nei, path, isFound);
				break;
			}
		}
	}
}


void FillPathDepth(Graph& graph, int first, int second, Componenta& comp)
{
	comp.path.push_back(first);
	comp.path.push_back(second);
	bool isFound = false;
	FindPathDepth(graph, second, comp.path, isFound);
}





void FindMultiEdgeComp(Graph& graph, int compStartVert, Componenta& comp)
{
	int faceNeigh = 0;
	int inner = 0;
	ClearBroadMarks(graph);
	faceNeigh = FindMultiEdgeVertx(graph, compStartVert);
	ClearBroadMarks(graph);
	graph.broadMark[faceNeigh] = 0;
	CompVert start(faceNeigh, true);
	comp.vers.push_back(start);
		
	for (size_t i = 0; i < graph.numNeigh[faceNeigh]; ++i) // заходим внутрь грани
	{
		int nei = graph.neighbours[faceNeigh][i];
		if (graph.inComp[nei] == 't')
		{
			graph.broadMark[nei] = 1;
			CompVert nextCV(nei, false);
			comp.vers.push_back(nextCV);
			inner = nei;
		}
	}
	int currMark = 1;

	bool newMarked = true;
	while (newMarked)
	{
		newMarked = false;
		for (size_t i = 1; i <= graph.numVert; ++i)
		{
			if ((graph.broadMark[i] == currMark) && (graph.inComp[i] == 't'))
			{
				for (size_t j = 0; j < graph.numNeigh[i]; ++j)
				{
					int nei = graph.neighbours[i][j];
					if (graph.broadMark[nei] == -1)
					{
						graph.broadMark[nei] = currMark + 1;
						bool cont = (graph.inComp[nei] == 'f') ? false : true; // контактная или нет
						CompVert nextCV(nei, cont);
						comp.vers.push_back(nextCV);
											
						newMarked = true;
					}
				}
			}
		}
		currMark++;
	}
	
	FillPathDepth(graph, faceNeigh, inner, comp);

	for (size_t k = 0; k < comp.vers.size(); ++k)
	{
		int cvert = comp.vers[k].name;
		if (graph.inComp[cvert] == 't') graph.inComp[cvert] = 'c';
	}
		
}



void AddMultiEdgeComps(Graph& graph, std::vector<Componenta>& comps)
{
	bool notComplete = true;
	while (notComplete)
	{
		notComplete = false;
		int compStartVert;
		for (size_t i = 1; i <= graph.numVert; ++i)
		{
			if (graph.inComp[i] == 'n')
			{
				compStartVert = i;
				notComplete = true;
				break;
			}
		}
		if (notComplete)
		{
			Componenta nextComp;
			ClearBroadMarks(graph);
			FindMultiEdgeComp(graph, compStartVert, nextComp);
			comps.push_back(nextComp);
		}
	}
}

void AddOneEdgeComps(Graph const& graph, std::vector<Componenta>& comps, std::vector<Face> const& faceSet)
{
	for (size_t i = 1; i <= graph.numVert; ++i)
	{
		if (graph.inComp[i] == 'f')
		{
			for (size_t j = i + 1; j <= graph.numVert; ++j)
			{
				if (graph.inComp[j] == 'f' && EdgeInGraph(graph, i, j))
				{
					bool notSide = true;
					for (size_t k = 0; k < faceSet.size(); ++k)
					{
						if (IsEdgeAFaceSide(i, j, faceSet[k]))
						{
							notSide = false;
							break;
						}
					}
					if (notSide)
					{
						Componenta oneEdge;
						CompVert vrt1(i, true);
						oneEdge.startVert = 0;
						oneEdge.vers.push_back(vrt1);
						oneEdge.path.push_back(i);
						CompVert vrt2(j, true);
						oneEdge.vers.push_back(vrt2);
						oneEdge.path.push_back(j);
						comps.push_back(oneEdge);
					}
				}
			}
		}
	}
}

bool IsCompInFace(Face const& face, Componenta const& comp) // лежит ли компонента comp в грани face
{
	bool result = true;
	for (auto& cv : comp.vers)
	{
		bool nexists = (find(face.vrtx.begin(), face.vrtx.end(), cv.name) == face.vrtx.end());
		if (cv.contact && nexists)
		{
			result = false;
			break;
		}
	}
	return result;
}

void SetCompInFace(std::vector<Face> const& faceSet, std::vector<Componenta>& comps)
{ // помещаем компоненты в грани
	for (size_t i = 0; i < comps.size(); ++i)
	{
		for (size_t j = 0; j < faceSet.size(); ++j)
		{
			if (IsCompInFace(faceSet[j], comps[i]))
			{
				comps[i].facs.push_back(j);
			}
		}
	}
}

int CompToInpose(std::vector<Componenta> const& comps) // компонента для дробления грани
{
	
	std::vector<int> times;
	for (size_t i = 0; i < comps.size(); ++i) times.push_back(comps[i].facs.size());
	
	auto it = min_element(times.begin(), times.end());
	int res = (times.empty()) ? -1 : std::distance(times.begin(), it);
	return res;
}

void TwoFacesFromOne(Graph& graph, Face const& iniFace, std::vector<int> const& newBoard, // разбиение грани на две
	Face& newFace1, Face& newFace2)
{
	auto it1 = find(iniFace.vrtx.begin(), iniFace.vrtx.end(), newBoard.front());
	auto it2 = find(iniFace.vrtx.begin(), iniFace.vrtx.end(), newBoard.back());
	
	if (it1 <= it2)
	{
		for (auto cit = iniFace.vrtx.begin(); cit != it1; ++cit) newFace1.vrtx.push_back(*cit);
		for (auto cit = newBoard.begin(); cit != newBoard.end() - 1; ++cit)
		{
			newFace1.vrtx.push_back(*cit);
			graph.inComp[*cit] = 'f';
		}			
		for (auto cit = it2; cit != iniFace.vrtx.end(); ++cit) newFace1.vrtx.push_back(*cit);	// построили грань newFace1
		
		for (auto cit = it1; cit != it2; ++cit) newFace2.vrtx.push_back(*cit);
		for (auto cit = newBoard.rbegin(); cit != newBoard.rend() - 1; ++cit) newFace2.vrtx.push_back(*cit); // постоили грань newFace2
	}
	else
	{
		for (auto cit = iniFace.vrtx.begin(); cit != it2; ++cit) newFace1.vrtx.push_back(*cit);
		for (auto cit = newBoard.rbegin(); cit != newBoard.rend() - 1; ++cit) 
		{
			newFace1.vrtx.push_back(*cit);
			graph.inComp[*cit] = 'f';
		}
		for (auto cit = it1; cit != iniFace.vrtx.end(); ++cit) newFace1.vrtx.push_back(*cit); 	// построили грань newFace1
		
		for (auto cit = it2; cit != it1; ++cit) newFace2.vrtx.push_back(*cit);
		for (auto cit = newBoard.begin(); cit != newBoard.end() - 1; ++cit) newFace2.vrtx.push_back(*cit); // постоили грань newFace2
	}
}



bool MakeFaces(Graph& graph, std::vector<Face>& faceSet) // умножение числа граней
{
	bool canDivide = true;
	bool compExists = true;

	InitFaceSet(graph, faceSet);
	int step = 0;

	while (canDivide && compExists)
	{
		std::cout << "\n*******************Iteration************************\n";
		ClearInCompMarks(graph);
		std::vector<Componenta> comps;
		AddOneEdgeComps(graph, comps, faceSet);
		AddMultiEdgeComps(graph, comps);

		compExists = (comps.size() != 0);
		PrintGraph(graph);
		PrintFaces(faceSet);

		std::cout << "Components to set:\n\n";
		for (size_t i = 0; i < comps.size(); ++i)
		{
			std::cout << "Number " << i;
			PrintComponenta(comps[i]);
			std::cout << std::endl;
		}
		
		SetCompInFace(faceSet, comps);
		int theComp = CompToInpose(comps);
		
		std::cout << "Componenta " << theComp << " is to be set\n";
		
		if (compExists)
		{
			if (comps[theComp].facs.empty())
			{
				std::cout << "Componenta " << theComp << " can't be imposed!" << std::endl;
				canDivide = false;
			}
			else
			{
				step++;
				int faceToDivide = comps[theComp].facs[0];
				std::cout << "Componenta is to be set into face " << faceToDivide << "\n";
				Face newFace1;
				Face newFace2;
				newFace1.step = step;
				newFace2.step = step;
				TwoFacesFromOne(graph, faceSet[faceToDivide], comps[theComp].path, newFace1, newFace2);
				faceSet[faceToDivide] = newFace1;
				faceSet.push_back(newFace2);
			}
		}
	}

	PrintGraph(graph);

	return canDivide;
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
		
		std::vector<Face> faceSet;
		
		bool isPlanar = MakeFaces(graph, faceSet);
		if (isPlanar)
		{
			std::cout << "Graph is planar. The faces are:\n";
			PrintFaces(faceSet);
		}
		else
		{
			std::cout << "Graph is nonplanar" << std::endl;
		}
				
	}

	return 0;
}




