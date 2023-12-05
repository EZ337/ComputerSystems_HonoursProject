# Options to set when compiling/linking the project.
CXXFLAGS=-g
LDFLAGS=

# The name of the executable to generate.
TARGET=client

# The objects that should be compiled from the project source files (expected
# to correspond to actual source files, e.g. URI.o will come from URI.cpp).
#
# You will want to add the name of your driver object to this list.
OBJS=	client.o\
	HTTP_Message.o \
	HTTP_Request.o \
	HTTP_Response.o \
	URI.o


# Have everything built automatically based on the above settings.
all: $(TARGET)

.o:
	g++ -o $@ $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	g++ -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJS)


# Dependencies follow (i.e. which source files and headers a given object is
# built from).
HTTP_Message.o: HTTP_Message.cpp HTTP_Message.h
HTTP_Request.o: HTTP_Request.cpp HTTP_Request.h HTTP_Message.h
HTTP_Response.o: HTTP_Response.cpp HTTP_Response.h HTTP_Message.h
URI.o: URI.cpp URI.h
