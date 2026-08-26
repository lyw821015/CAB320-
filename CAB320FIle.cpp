import search 
import sokoban

# I changed a bit of the numbers so that it shows the up, down, left and right accordingly and capitalized the
# names so that it matches with the standard Sokoban action name
directions = {
    'Up': (0, -1),
    'Down' : (0, 1),
    'Left' : (-1, 0),
    'Right' : (1, 0),
}

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

def my_team() :

    return[(12418161, 'Savitha', 'Perera'), (12708437, 'Yeowon', 'Lim')]

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    def taboo_cells(warehouse) :
    
    walls = set(warehouse.walls)
    targets = set(warehouse.targets)
    X_MAX = warehouse.ncols
    Y_MAX = warehouse.nrows

    # Step 1: Flood - fill
    border_cells = (
        [(x, y) for x in range(X_MAX) for y in(0, Y_MAX - 1)]
        + [(x, y) for y in range(1, Y_MAX - 1) for x in(0, X_MAX - 1)]
        )

    outside = set()
    queue = [c for c in border_cells if c not in walls]
    visited = set(queue)

    while queue:
x, y = queue.pop()
outside.add((x, y))
for dx, dy in directions.values() :
    nx, ny = x + dx, y + dy
    nb = (nx, ny)
    if (0 <= nx < X_MAX and 0 <= ny < Y_MAX
        and nb not in visited and nb not in walls) :
        visited.add(nb)
        queue.append(nb)

        inside = {
            (x, y)
            for y in range(Y_MAX)
            for x in range(X_MAX)
            if (x, y) not in walls and (x, y) not in outside
    }

        # Step 2: Rule 1
        corners = {
            (x, y)
            for (x, y) in inside
            if ((x - 1, y) in walls or (x + 1, y) in walls)
            and ((x, y - 1) in walls or (x, y + 1) in walls)
    }

        taboo = set()
        taboo |= corners - targets

        # Rule 2: Horizontal spans
        for y in range(Y_MAX) :
            row_corner_xs = sorted(x for (x, cy) in corners if cy == y)
            for i in range(len(row_corner_xs)) :
                for j in range(i + 1, len(row_corner_xs)) :
                    x1, x2 = row_corner_xs[i], row_corner_xs[j]
                    between = [(x, y) for x in range(x1 + 1, x2)]
                    if not between : continue
                    if not all(c in inside for c in between) : continue
                    if any(c in targets for c in between) : continue
                    span_xs = range(x1, x2 + 1)
                    if (all((x, y - 1) in walls for x in span_xs)
                           or all((x, y + 1) in walls for x in span_xs)) :
                       taboo.update(between)

        # Rule 2: Vertical spans
        for x in range(X_MAX) :
            col_corner_ys = sorted(y for (cx, y) in corners if cx == x)
            for i in range(len(col_corner_ys)) :
                for j in range(i + 1, len(col_corner_ys)) :
                    y1, y2 = col_corner_ys[i], col_corner_ys[j]
                    between = [(x, y) for y in range(y1 + 1, y2)]
                    if not between : continue
                    if not all(c in inside for c in between) : continue
                    if any(c in targets for c in between) : continue
                    span_ys = range(y1, y2 + 1)
                    if (all((x - 1, y) in walls for y in span_ys)
                            or all((x + 1, y) in walls for y in span_ys)) :
                       taboo.update(between)

        X_coords, Y_coords = zip(*warehouse.walls)
        x_size = 1 + max(X_coords)
        y_size = 1 + max(Y_coords)

        vis = [[' ']* x_size for _ in range(y_size)]
        for (x, y) in warehouse.walls: vis[y][x] = '#'
        for (x, y) in taboo : vis[y][x] = 'X'

       return '\n'.join(''.join(row) for row in vis)
# I removed the NotImplementError so that it keeps on working just in case. But Im not sure of this
# So you can edit if you want

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        class SokobanPuzzle(search.Problem) :
        def __init__(self, warehouse) :
        self.walls = frozenset(warehouse.walls)
        self.targets = frozenset(warehouse.targets)
        self.weights = tuple(warehouse.weights)
        taboo_str = taboo_cells(warehouse)

#I fixed logic to parse taboo cells from the string result into a frozenset
#to ensure efficient lookup during move validation.
        self.taboo = set()
        for y, line in enumerate(taboo_str.split('\n')) :
            for x, ch in enumerate(line) :
                if ch == 'X' : self.taboo.add((x, y))
        self.taboo = frozenset(self.taboo)
        initial_state = (warehouse.worker, tuple(warehouse.boxes))

#I used the super code and removed NotImplementedError 
        super().__init__(initial_state)

     def actions(self, state) :
        worker, boxes = state
        boxes_set = set(boxes)
        legal = []
        for action, (dx, dy) in directions.items() :
            nw = (worker[0] + dx, worker[1] + dy)
            if nw in self.walls : continue
            if nw in boxes_set :
                nb = (nw[0] + dx, nw[1] + dy)
                if nb in self.walls or nb in boxes_set or nb in self.taboo :
                      continue
            legal.append(action)
        return legal

  def result(self, state, action) :
    worker, boxes = state
    dx, dy = directions[action]
    nw = (worker[0] + dx, worker[1] + dy)
    boxes_list = list(boxes)
    if nw in set(boxes) :
        idx = boxes_list.index(nw)
        boxes_list[idx] = (nw[0] + dx, nw[1] + dy)
    return (nw, tuple(boxes_list))

   def goal_test(self, state) :
        _, boxes = state
        return set(boxes) == self.targets

   def path_cost(self, c, state1, action, state2) :
        worker, boxes1 = state1
        dx, dy = directions[action]
        nw = (worker[0] + dx, worker[1] + dy)
        if nw in set(boxes1) :
            idx = list(boxes1).index(nw)
            return c + 1 + self.weights[idx]
        return c + 1

# I used the admissible weighted manhattan distance heuristic.
# So that it calculates the lower bound of cost by scaling distance by box weight 
   def h(self, node) :
    _, boxes = node.state
    targets = list(self.targets)
    total = 0
    for i, box in enumerate(boxes) :
        if box not in self.targets :
            min_dist = min(abs(box[0] - t[0]) + abs(box[1] - t[1]) for t in targets)
            total += (1 + self.weights[i]) * min_dist
    return total

## removed NotImplementedError from h()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   def check_elem_action_seq(warehouse, action_seq) :
     worker = warehouse.worker
     boxes = list(warehouse.boxes)
     walls = set(warehouse.walls)

for action in action_seq :
    if action not in directions : return 'Impossible'
    dx, dy = directions[action]
    nw = (worker[0] + dx, worker[1] + dy)
    if nw in walls : return 'Impossible'
    boxes_set = set(boxes)
    if nw in boxes_set :
        nb = (nw[0] + dx, nw[1] + dy)
        if nb in walls or nb in boxes_set : return 'Impossible'
        idx = boxes.index(nw)
        boxes[idx] = nb
     worker = nw

# I returned the warehouse __str__() instead of raising an error.
result_wh = warehouse.copy(worker = worker, boxes = tuple(boxes))
return result_wh.__str__()
#removed NotImplementedError from check_elem_action_seq

#Added logic to call astar_graph_search and return solution & path_cost.
#returns 'Impossible', None if no solution is found.
def solve_weighted_sokoban(warehouse) :
    puzzle = SokobanPuzzle(warehouse)
    if puzzle.goal_test(puzzle.initial) :
        return[], 0
     node = search.astar_graph_search(puzzle, puzzle.h)
    if node is None :
        return 'Impossible', None
return node.solution(), node.path_cost
#removed NotImplementedError from solve_weighted_sokoban