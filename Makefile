EVOX11_LIBS := -lX11 -lXext -lXdamage -lXcomposite -lXrender -lcairo -lXfixes -ldl -lm
APP_NAME := EvoX11

all:
	gcc ConfGen.h EvoX11.c $(EVOX11_LIBS) -o $(APP_NAME)

install:
	cp ./EvoX11 /usr/bin
