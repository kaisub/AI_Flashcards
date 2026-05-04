#!/bin/bash

# Searching for a container whose IMAGE (ancestor) has the name ai_flashcards
CONTAINER_ID=$(docker ps --filter "ancestor=vsc-ai_flashcards-f23afe0c2c2d8aecff3c2b0ffcb869431c01d99654e94c361caeb29f425e2732-uid" --format "{{.ID}}" | head -n 1)

# If the above is too specific, use a broader filter by image name:
if [ -z "$CONTAINER_ID" ]; then
    CONTAINER_ID=$(docker ps --filter "ancestor=vsc-ai_flashcards" --format "{{.ID}}" | head -n 1)
fi

if [ -z "$CONTAINER_ID" ]; then
    echo "❌ No running container found for the ai_flashcards project."
    exit 1
fi

echo "🚀 Entering container: $CONTAINER_ID..."
docker exec -it "$CONTAINER_ID" /bin/bash