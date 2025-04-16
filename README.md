# [Guided Reader](https://reader.scarlettparker.co.uk)

## Overview

**Guided Reader** is a highly performant and responsive annotation tool that allows users to explore and annotate texts from τράπεζα κειμένων ([Greek Text Bank](https://www.greek-language.gr/certification/dbs/teachers/index.html)) in a style similar to Genius.com.

The application is built with:
- **SolidJS** for the front-end.
- **C++** for the back-end.

## Features

- **Text annotation**: Annotate texts interactively, with a word reference button that uses the [wordreference](https://www.npmjs.com/package/wordreference) npm package to display Greek-to-English dictionary entries for words.
- **Authentication via Discord OAuth2**.
- **Integration with PostgreSQL and Redis** for data and session management.

---

## Getting Started

### Front-End Setup

#### Prerequisites
- Node.js

#### Running the Front-End
1. Navigate to the front-end directory:
   ```bash
   cd client
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Start the development server:
   ```bash
   npm run dev
   ```
4. To build for production:
   ```bash
   npm run build
   ```
5. Serve the production build:
   ```bash
   npm run serve
   ```

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Additional Information
- Discord Guild ID for the Greek Learning Server: **350234668680871946**
- Discord API Endpoints:
  - **User Info:** `/api/users/@me`
  - **User Guilds:** `/api/users/@me/guilds`
  - **Token:** `/api/oauth2/token`
- Discord OAuth2 Permissions:
  - identify, guilds, guilds.members.read
- Discord Redirect URI: **client/auth_discord**
- Discord Link Redirect URI: **client/auth_discord?verify=true**

For more details, visit the website at [reader.scarlettparker.co.uk](https://reader.scarlettparker.co.uk).

