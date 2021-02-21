
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define BSC_M 64

#define BSC_X 128
#define BSC_Y 64
#define BSC_Z 64

struct cube
{
	int *w_floor;
	struct w_node **w_axis;
	unsigned char *x_size;
	unsigned short w_size;
};

struct w_node
{
	int x_floor[BSC_X];
	struct x_node *x_axis[BSC_X];
	unsigned char y_size[BSC_X];
};

struct x_node
{
	int y_floor[BSC_Y];
	struct y_node *y_axis[BSC_Y];
	unsigned char z_size[BSC_Y];
};

struct y_node
{
	int z_keys[BSC_Z];
};

void split_w_node(struct cube *cube, short w);
void split_x_node(struct cube *cube, short w, short x);
void split_y_node(struct w_node *w_node, short x, short y);

struct cube *create_cube(void)
{
	struct cube *cube;

	cube = (struct cube *) calloc(1, sizeof(struct cube));

	return cube;
}

void destroy_cube(struct cube *cube, int *z_array)
{
	if (cube->w_size)
	{
		struct w_node *w_node;
		struct x_node *x_node;
		struct y_node *y_node;

		register short w, x, y;

		register int z_size = 0;

		for (w = 0 ; w < cube->w_size ; ++w)
		{
			w_node = cube->w_axis[w];

			for (x = 0 ; x < cube->x_size[w] ; ++x)
			{
				x_node = w_node->x_axis[x];
			
				for (y = 0 ; y < w_node->y_size[x] ; ++y)
				{
					y_node = x_node->y_axis[y];

					memmove(&z_array[z_size], &y_node->z_keys[0], x_node->z_size[y] * sizeof(int));

					z_size += x_node->z_size[y];

					free(y_node);
				}
				free(x_node);
			}
			free(w_node);
		}
		free(cube->w_floor);
		free(cube->w_axis);
		free(cube->x_size);
	}
	free(cube);
}

inline void insert_z_node(struct cube *cube, int key)
{
	struct w_node *w_node;
	struct x_node *x_node;
	struct y_node *y_node;

	register short mid, w, x, y, z;

	if (cube->w_size == 0)
	{
		cube->w_floor = (int *) malloc(BSC_M * sizeof(int));
		cube->w_axis = (struct w_node **) malloc(BSC_M * sizeof(struct w_node *));
		cube->x_size = (unsigned char *) malloc(BSC_M * sizeof(unsigned char));

		w_node = cube->w_axis[0] = (struct w_node *) malloc(sizeof(struct w_node));

		x_node = w_node->x_axis[0] = (struct x_node *) malloc(sizeof(struct x_node));

		y_node = x_node->y_axis[0] = (struct y_node *) malloc(sizeof(struct y_node));

		x_node->z_size[0] = 0;

		cube->w_size = cube->x_size[0] = w_node->y_size[0] = 1;

		w = x = y = z = 0;
		
		cube->w_floor[0] = w_node->x_floor[0] = x_node->y_floor[0] = key;

		goto insert;
	}

	if (key < cube->w_floor[0])
	{
		w_node = cube->w_axis[0];
		x_node = w_node->x_axis[0];
		y_node = x_node->y_axis[0];

		w = x = y = z = 0;

		cube->w_floor[0] = w_node->x_floor[0] = x_node->y_floor[0] = key;

		goto insert;
	}

	// w

	mid = w = cube->w_size - 1;

	while (mid > 7)
	{
		mid /= 2;

		if (key < cube->w_floor[w - mid]) w -= mid;
	}
	while (key < cube->w_floor[w]) --w;

	w_node = cube->w_axis[w];

	// x

	mid = x = cube->x_size[w] - 1;

	while (mid > 7)
	{
		mid /= 2;

		if (key < w_node->x_floor[x - mid])
		{
			x -= mid;
		}
	}
	while (key < w_node->x_floor[x]) --x;

	x_node = w_node->x_axis[x];

	// y

	mid = y = w_node->y_size[x] - 1;

	while (mid > 7)
	{
		mid /= 4;

		if (key < x_node->y_floor[y - mid])
		{
			y -= mid;
			if (key < x_node->y_floor[y - mid])
			{
				y -= mid;
				if (key < x_node->y_floor[y - mid])
				{
					y -= mid;
				}
			}
		}
	}
	while (key < x_node->y_floor[y]) --y;

	y_node = x_node->y_axis[y];

	// z

	mid = z = x_node->z_size[y] - 1;

	while (mid > 7)
	{
		mid /= 4;

		if (key < y_node->z_keys[z - mid])
		{
			z -= mid;
			if (key < y_node->z_keys[z - mid])
			{
				z -= mid;
				if (key < y_node->z_keys[z - mid])
				{
					z -= mid;
				}
			}
		}
	}
	while (key < y_node->z_keys[z]) --z;


/*
//	Uncomment to filter duplicates

	if (key == y_node->z_keys[z])
	{
		return;
	}
*/
	++z;

	insert:

	++x_node->z_size[y];

	if (z + 1 != x_node->z_size[y])
	{
		memmove(&y_node->z_keys[z + 1], &y_node->z_keys[z], (x_node->z_size[y] - z - 1) * sizeof(int));
	}

	y_node->z_keys[z] = key;

	if (x_node->z_size[y] == BSC_Z)
	{
		split_y_node(w_node, x, y);

		if (w_node->y_size[x] == BSC_Y)
		{
			split_x_node(cube, w, x);

			if (cube->x_size[w] == BSC_X)
			{
				split_w_node(cube, w);
			}
		}
	}
}

inline void insert_w_node(struct cube *cube, short w)
{
	++cube->w_size;

	if (cube->w_size % BSC_M == 0)
	{
		cube->w_floor = (int *) realloc(cube->w_floor, (cube->w_size + BSC_M) * sizeof(int));
		cube->w_axis = (struct w_node **) realloc(cube->w_axis, (cube->w_size + BSC_M) * sizeof(struct w_node *));
		cube->x_size = (unsigned char *) realloc(cube->x_size, (cube->w_size + BSC_M) * sizeof(unsigned char));
	}

	if (w + 1 != cube->w_size)
	{
		memmove(&cube->w_floor[w + 1], &cube->w_floor[w], (cube->w_size - w - 1) * sizeof(int));
		memmove(&cube->w_axis[w + 1], &cube->w_axis[w], (cube->w_size - w - 1) * sizeof(struct w_node *));
		memmove(&cube->x_size[w + 1], &cube->x_size[w], (cube->w_size - w - 1) * sizeof(unsigned char));
	}

	cube->w_axis[w] = (struct w_node *) malloc(sizeof(struct w_node));
}

void split_w_node(struct cube *cube, short w)
{
	struct w_node *w_node1, *w_node2;

	insert_w_node(cube, w + 1);

	w_node1 = cube->w_axis[w];
	w_node2 = cube->w_axis[w + 1];

	cube->x_size[w + 1] = cube->x_size[w] / 2;
	cube->x_size[w] -= cube->x_size[w + 1];

	memcpy(&w_node2->x_floor[0], &w_node1->x_floor[cube->x_size[w]], cube->x_size[w + 1] * sizeof(int));
	memcpy(&w_node2->x_axis[0], &w_node1->x_axis[cube->x_size[w]], cube->x_size[w + 1] * sizeof(struct x_node *));
	memcpy(&w_node2->y_size[0], &w_node1->y_size[cube->x_size[w]], cube->x_size[w + 1] * sizeof(unsigned char));

	cube->w_floor[w + 1] = w_node2->x_floor[0];
}

inline void insert_x_node(struct cube *cube, short w, short x)
{
	struct w_node *w_node = cube->w_axis[w];

	short x_size = ++cube->x_size[w];

	if (x_size != x + 1)
	{
		memmove(&w_node->x_floor[x + 1], &w_node->x_floor[x], (x_size - x - 1) * sizeof(int));
		memmove(&w_node->x_axis[x + 1], &w_node->x_axis[x], (x_size - x - 1) * sizeof(struct x_node *));
		memmove(&w_node->y_size[x + 1], &w_node->y_size[x], (x_size - x - 1) * sizeof(unsigned char));
	}

	w_node->x_axis[x] = (struct x_node *) malloc(sizeof(struct x_node));
}

void split_x_node(struct cube *cube, short w, short x)
{
	struct w_node *w_node = cube->w_axis[w];
	struct x_node *x_node1, *x_node2;

	insert_x_node(cube, w, x + 1);

	x_node1 = w_node->x_axis[x];
	x_node2 = w_node->x_axis[x + 1];

	w_node->y_size[x + 1] = w_node->y_size[x] / 2;
	w_node->y_size[x] -= w_node->y_size[x + 1];

	memcpy(&x_node2->y_floor[0], &x_node1->y_floor[w_node->y_size[x]], w_node->y_size[x + 1] * sizeof(int));
	memcpy(&x_node2->y_axis[0], &x_node1->y_axis[w_node->y_size[x]], w_node->y_size[x + 1] * sizeof(struct y_node *));
	memcpy(&x_node2->z_size[0], &x_node1->z_size[w_node->y_size[x]], w_node->y_size[x + 1] * sizeof(unsigned char));

	w_node->x_floor[x + 1] = x_node2->y_floor[0];
}

inline void insert_y_node(struct w_node *w_node, short x, short y)
{
	struct x_node *x_node = w_node->x_axis[x];

	short y_size = ++w_node->y_size[x];

	if (y_size != y + 1)
	{
		memmove(&x_node->y_floor[y + 1], &x_node->y_floor[y], (y_size - y - 1) * sizeof(int));
		memmove(&x_node->y_axis[y + 1], &x_node->y_axis[y], (y_size - y - 1) * sizeof(struct y_node *));
		memmove(&x_node->z_size[y + 1], &x_node->z_size[y], (y_size - y - 1) * sizeof(unsigned char));
	}

	x_node->y_axis[y] = (struct y_node *) malloc(sizeof(struct y_node));
}

void split_y_node(struct w_node *w_node, short x, short y)
{
	struct x_node *x_node = w_node->x_axis[x];
	struct y_node *y_node1, *y_node2;

	insert_y_node(w_node, x, y + 1);

	y_node1 = x_node->y_axis[y];
	y_node2 = x_node->y_axis[y + 1];

	x_node->z_size[y + 1] = x_node->z_size[y] / 2;
	x_node->z_size[y] -= x_node->z_size[y + 1];

	memcpy(&y_node2->z_keys[0], &y_node1->z_keys[x_node->z_size[y]], x_node->z_size[y + 1] * sizeof(int));

	x_node->y_floor[y + 1] = y_node2->z_keys[0];
}

long long utime()
{
	struct timeval now_time;

	gettimeofday(&now_time, NULL);

	return now_time.tv_sec * 1000000LL + now_time.tv_usec;
}

void cubesort(int *array, int size)
{
	struct cube *cube;
	int cnt;

	if (size > 1000000)
	{
		for (cnt = 100000 ; cnt + 100000 < size ; cnt += 100000)
		{
			cubesort(&array[cnt], 100000);
		}

		cubesort(&array[cnt], size - cnt);
	}

	cube = create_cube();

	for (cnt = 0 ; cnt < size ; cnt++)
	{
		insert_z_node(cube, array[cnt]);
	}
	destroy_cube(cube, array);
}

int main(int argc, char **argv)
{
	static int max = 10000000;
	int cnt;
	long long start, end;
	int *z_array;

	if (argv[1] && *argv[1])
	{
		printf("%s\n", argv[1]);
	}

	//z_array = malloc(max * sizeof(int));
	z_array = (int *)malloc(max * sizeof(int));

	for (cnt = 0 ; cnt < max ; cnt++)
	{
		z_array[cnt] = rand();
	}

	start = utime();

	cubesort(z_array, max);

	end = utime();

	printf("Cubesort: sorted %d elements in %f seconds. (random order)\n", max, (end - start) / 1000000.0);

	for (cnt = 0 ; cnt < max ; cnt++)
	{
		z_array[cnt] = cnt;
	}

	start = utime();

	cubesort(z_array, max);

	end = utime();

	printf("Cubesort: sorted %d elements in %f seconds. (forward order)\n", max, (end - start) / 1000000.0);

	for (cnt = 0 ; cnt < max ; cnt++)
	{
		z_array[cnt] = max - cnt;
	}

	start = utime();

	cubesort(z_array, max);

	end = utime();

	printf("Cubesort: sorted %d elements in %f seconds. (reverse order)\n", max, (end - start) / 1000000.0);

	for (cnt = 1 ; cnt < max ; cnt++)
	{
		if (z_array[cnt - 1] > z_array[cnt])
		{
			printf("Cubesort: not properly sorted at index %d. (%d vs %d\n", cnt, z_array[cnt - 1], z_array[cnt]);
			return 0;
		}
	}

	return 0;
}
