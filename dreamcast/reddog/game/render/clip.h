case CLIPCASE(0):		/* Triangle is completely offscreen */
	break;

case CLIPCASE(1):
	EMIT_EDGE (AA, BB);
	EMIT_EDGE (CC, AA);
	EMIT_FINAL(AA);
	break;
	
case CLIPCASE(2):
	EMIT_EDGE (BB, CC);
	EMIT_EDGE (AA, BB);
	EMIT_FINAL(BB);
	break;
	
case CLIPCASE(3):
	EMIT_EDGE (CC, BB);
	EMIT_EDGE(CC, AA);
	EMIT	  (BB);
	EMIT_FINAL (AA);
	break;
	
case CLIPCASE(4):
	EMIT_EDGE(AA, CC);
	EMIT_EDGE(BB, CC);
	EMIT_FINAL(CC);
	break;
	
case CLIPCASE(5):
	EMIT_EDGE (AA, BB);
	EMIT_EDGE (BB, CC);
	EMIT (AA);
	EMIT_FINAL(CC);
	break;
	
case CLIPCASE(6):
	EMIT_EDGE(AA, CC);
	EMIT_EDGE(AA, BB);
	EMIT	  (CC);
	EMIT_FINAL(BB);
	break;
	
case CLIPCASE(7):
	EMIT(AA);
	EMIT(BB);
	EMIT_FINAL(CC);
	break;
