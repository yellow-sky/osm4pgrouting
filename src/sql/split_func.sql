CREATE OR REPLACE FUNCTION temp_sp_splitroads()
RETURNS int4 AS
$$
DECLARE
	way 		RECORD; --active way
	back_node	RECORD; --last point in way
	it_node		RECORD;	--active node
	way_tag_rec	RECORD; --temporary tag rec
	point_rec	RECORD;	--temporary point rec
	
	first_point 	geometry;
	second_point	geometry;
	segment 	geometry;

	start_point 	geometry;
	end_point	geometry;
 
	id		int4;
	oneway 		int4;
	length		float;
	to_cost		float;
	rev_cost	float;
BEGIN
	id=0;
	
	-- iterate throw the ways
	FOR way IN (SELECT * FROM %TP%temp_ways) LOOP		
 
		--get last point
		SELECT * FROM %TP%temp_way_node
		WHERE way_id = way.osm_id
		ORDER BY id DESC LIMIT 1 
		INTO back_node;
 
		--get last point
		SELECT * FROM %TP%temp_way_node
		WHERE way_id = way.osm_id
		ORDER BY id ASC LIMIT 1 
		INTO it_node;
 
		--iterate throw the nodes
		WHILE it_node.id <> back_node.id LOOP
	
			--RAISE NOTICE '===NEW LINE!==='; --test
 
			--get first point in segment
			SELECT * FROM %TP%nodes WHERE %TP%nodes.id=it_node.node_id INTO point_rec;
			first_point = ST_MakePoint(point_rec.lon, point_rec.lat);
			
			--get second
				   SELECT * FROM %TP%temp_way_node 
				   WHERE %TP%temp_way_node.way_id = way.osm_id AND %TP%temp_way_node.id > it_node.id 
				   ORDER BY id ASC LIMIT 1
				   INTO it_node;
			SELECT * FROM %TP%nodes WHERE %TP%nodes.id=it_node.node_id INTO point_rec;
			second_point = ST_MakePoint(point_rec.lon, point_rec.lat);
 
			--create LINE
			segment = ST_SetSRID(ST_MakeLine(first_point, second_point), 4326);
			--RAISE NOTICE 'Line: %', ST_AsText(segment); --test
			
			WHILE it_node.id <> back_node.id AND it_node.num_of_use < 2 LOOP
				   --get next
				   SELECT * FROM %TP%temp_way_node 
				   WHERE %TP%temp_way_node.way_id = way.osm_id AND %TP%temp_way_node.id > it_node.id 
				   ORDER BY id ASC LIMIT 1
				   INTO it_node;
				   --add point to LINE
				   SELECT * FROM %TP%nodes WHERE %TP%nodes.id=it_node.node_id INTO point_rec;
				   segment = ST_AddPoint(segment, ST_MakePoint(point_rec.lon, point_rec.lat));
				  --RAISE NOTICE 'Line: %', ST_AsText(segment); --test
			END LOOP;
 
			--create way
			--OneWayType{ NO=0 , YES=1, REVERSED=2}
			length = ST_Length(segment);
			IF (way.oneway=0) THEN	
				to_cost = length;
				rev_cost = length;
			ELSIF (way.oneway=1) THEN  
				to_cost = length;
				rev_cost = length*1000000;
			ELSE
				to_cost = length*1000000;
				rev_cost = length;
			END IF;	

			--x1-y2 (by osm2pgrouting WTF???)
			start_point = ST_StartPoint(segment); --first_point!!!
			end_point = ST_EndPoint(segment);
						
			INSERT INTO %TP%ways (gid, class_id, length, x1, y1, x2, y2, osm_id, the_geom, to_cost, reverse_cost, maxspeed_forward, maxspeed_backward, priority, name)
				       VALUES (id, way.class_id, length, ST_X(start_point), ST_Y(start_point), ST_X(end_point), ST_Y(end_point), way.osm_id, segment, to_cost, rev_cost, way.maxspeed_forward, way.maxspeed_backward, way.priority, way.name); --test
 
			--copy tags
			FOR way_tag_rec IN (SELECT * FROM %TP%temp_way_tag where way_id=way.osm_id) LOOP
				INSERT INTO %TP%way_tag (type_id, class_id, way_id)
				VALUES (way_tag_rec.type_id, way_tag_rec.class_id, id);
			END LOOP;
			id=id+1;
		END LOOP;
 
		IF mod(id,100)=0 THEN
			RAISE NOTICE 'Created: %', id;
		END IF;
	END LOOP;
 
	RETURN id;
END;
$$
LANGUAGE plpgsql;
 
SELECT temp_sp_splitroads();

DROP FUNCTION IF EXISTS temp_sp_splitroads();