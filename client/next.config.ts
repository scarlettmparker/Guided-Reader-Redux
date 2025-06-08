import { NextConfig } from 'next';
import createNextIntlPlugin from 'next-intl/plugin';

const withNextIntl = createNextIntlPlugin();

const nextConfig: NextConfig = {
  async rewrites() {
    const host = process.env.SERVER_HOST;
    const port = process.env.SERVER_PORT;

    if (!host || !port) {
      console.warn('Missing SERVER_HOST or SERVER_PORT environment variables.');
      return [];
    }

    const destination = `http://${host}:${port}/:path*`;

    return [
      {
        source: '/api/:path*',
        destination: `http://${host}:${port}/:path*`,
      },
    ];
  },
};

export default withNextIntl(nextConfig);
