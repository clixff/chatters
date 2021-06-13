import { Server } from  "./server";
import SocketsServer from './sockets';

const server = new Server();

server.setRoutes();

export let socketsServer: SocketsServer | null = null;

if (server.Instance)
{
    socketsServer = new SocketsServer(server.Instance.server);
}

server.listen();
