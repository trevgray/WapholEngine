#include "NavigationMesh.h"
#include "EngineManager.h"
#include "MaterialComponent.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"
#include "jc_voronoi_clip.h"

NavigationMesh::NavigationMesh() {
	//memset(&diagram, 0, sizeof(jcv_diagram));
	//voronoiDiagram = std::make_shared<jcv_diagram>();
}

NavigationMesh::~NavigationMesh() {
	//jcv_diagram_free(&diagram);
}

void NavigationMesh::Initialize(MATH::Vec3 bottomLeftCorner, MATH::Vec3 topRightCorner, std::vector<std::string> ignoreActors) {
	//JCV Voronoi Generation
	Ref<jcv_diagram> jcvVoronoi;
	jcvVoronoi = std::make_shared<jcv_diagram>();

	jcv_rect* rect = new jcv_rect();

	jcv_point* borders = (jcv_point*)malloc(sizeof(jcv_point) * (size_t)2);
	borders[0].x = bottomLeftCorner.x;
	borders[0].y = bottomLeftCorner.y;

	borders[1].x = topRightCorner.x;
	borders[1].y = topRightCorner.y;

	rect->min = jcv_point(borders[0]);
	rect->max = jcv_point(borders[1]);
	
	std::vector<Vec3> actorPositions;
	bool skipActor;

	for (auto actor : EngineManager::Instance()->GetActorManager()->GetActorGraph()) {
		if (actor.second->GetComponent<MaterialComponent>() != nullptr) { //everything is an actor, so i just check if it has a texture
			skipActor = false;
			std::cout << "BRUH" << std::endl;
			for (std::string ignoreActorName : ignoreActors) {
				if (actor.first == ignoreActorName) {
					skipActor = true;
				}
			}
			if (skipActor == true) {
				continue;
			}
			actorPositions.push_back(actor.second->GetComponent<TransformComponent>()->GetPosition());
		}
	}

	int num_points = actorPositions.size();

	jcv_point* points = 0;
	points = (jcv_point*)malloc(sizeof(jcv_point) * (size_t)num_points);

	int pointIterator = 0;

	for (Vec3 point : actorPositions) {
		points[pointIterator].x = point.x;
		points[pointIterator].y = point.y;
		pointIterator++;
	}

	/*points[0].x = 0.0f;
	points[0].y = -5.0f;

	points[1].x = 5.0f;
	points[1].y = 20.0f;

	points[2].x = 10.0f;
	points[2].y = -5.0f;

	points[3].x = -10.0f;
	points[3].y = 5.0f;

	points[4].x = 20.0f;
	points[4].y = 5.0f;

	points[5].x = 5.0f;
	points[5].y = 5.0f;

	points[6].x = -15.0f;
	points[6].y = 10.0f;*/

	jcv_clipper* clipper = 0;

	jcv_diagram_generate(num_points, points, rect, clipper, jcvVoronoi.get());
	
	//TURN THE JCV EDGES AND POINTS INTO GRAPH POINTS
	std::vector<Node> graphNodes;
	int nodeLabel = 0;
	//push nodes on the graph and create the graph
	const jcv_edge* edge = jcv_diagram_get_edges(jcvVoronoi.get());
	while (edge) {
		graphNodes.push_back(Node(nodeLabel, MATH::Vec3(edge->pos[0].x, edge->pos[0].y, 0)));
		nodeLabel++;
		//std::cout << edge->pos[0].x << " " << edge->pos[0].y << " | " << edge->pos[1].x << " " << edge->pos[1].y << std::endl;
		edge = jcv_diagram_get_next_edge(edge);
	}

	voronoiGraph.OnCreate(graphNodes);

	 //Create connections
	edge = jcv_diagram_get_edges(jcvVoronoi.get());
	while (edge) {
		//std::cout << edge->pos[0].x << " " << edge->pos[0].y << " | " << edge->pos[1].x << " " << edge->pos[1].y << std::endl;
		voronoiGraph.AddConnectionVector(Vec3(edge->pos[0].x, edge->pos[0].y, 0.0f), Vec3(edge->pos[1].x, edge->pos[1].y, 0.0f));
		edge = jcv_diagram_get_next_edge(edge);
	}

	//CLEAN UP THE JCV Voronoi Generation
	jcv_diagram_free(jcvVoronoi.get());

	if (clipper) { delete clipper; }
	if (rect) { delete rect; }

	free(points); //used malloc
	free(borders);
}